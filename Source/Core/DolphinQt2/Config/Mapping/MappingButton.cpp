// Copyright 2017 Dolphin Emulator Project
// Licensed under GPLv2+
// Refer to the license.txt file included.

#include <thread>

#include <QMouseEvent>
#include <QRegExp>
#include <QString>

#include "DolphinQt2/Config/Mapping/MappingButton.h"

#include "Common/Thread.h"
#include "DolphinQt2/Config/Mapping/IOWindow.h"
#include "DolphinQt2/Config/Mapping/MappingCommon.h"
#include "DolphinQt2/Config/Mapping/MappingWidget.h"
#include "DolphinQt2/Config/Mapping/MappingWindow.h"
#include "InputCommon/ControlReference/ControlReference.h"
#include "InputCommon/ControllerEmu/ControllerEmu.h"
#include "InputCommon/ControllerInterface/ControllerInterface.h"
#include "InputCommon/ControllerInterface/Device.h"

static QString EscapeAmpersand(QString&& string)
{
  return string.replace(QStringLiteral("&"), QStringLiteral("&&"));
}

MappingButton::MappingButton(MappingWidget* widget, ControlReference* ref)
    : ElidedButton(EscapeAmpersand(QString::fromStdString(ref->expression))), m_parent(widget),
      m_reference(ref)
{
  Connect();
}

void MappingButton::Connect()
{
  connect(this, &MappingButton::clicked, this, &MappingButton::OnButtonPressed);
}

void MappingButton::OnButtonPressed()
{
  if (m_parent->GetDevice() == nullptr || !m_reference->IsInput())
    return;

  if (!m_block.TestAndSet())
    return;

  grabKeyboard();
  grabMouse();

  // Make sure that we don't block event handling
  std::thread([this] {
    const auto dev = m_parent->GetDevice();

    setText(QStringLiteral("..."));

    // Avoid that the button press itself is registered as an event
    Common::SleepCurrentThread(100);

    const auto expr = MappingCommon::DetectExpression(m_reference, dev.get(),
                                                      m_parent->GetParent()->GetDeviceQualifier(),
                                                      m_parent->GetController()->default_device);

    releaseMouse();
    releaseKeyboard();
    m_block.Clear();

    if (!expr.isEmpty())
    {
      m_reference->expression = expr.toStdString();
      Update();
    }
    else
    {
      OnButtonTimeout();
    }
  }).detach();
}

void MappingButton::OnButtonTimeout()
{
  setText(EscapeAmpersand(QString::fromStdString(m_reference->expression)));
}

void MappingButton::Clear()
{
  m_parent->Update();
  m_reference->expression.clear();
  Update();
}

void MappingButton::Update()
{
  const auto lock = ControllerEmu::EmulatedController::GetStateLock();
  m_reference->UpdateReference(g_controller_interface, m_parent->GetParent()->GetDeviceQualifier());
  setText(EscapeAmpersand(QString::fromStdString(m_reference->expression)));
  m_parent->SaveSettings();
}

bool MappingButton::event(QEvent* event)
{
  const QEvent::Type event_type = event->type();
  // Returning 'true' means "yes, this event has been handled, don't propagate it to parent
  // widgets".
  if (m_block.IsSet() &&
      (event_type == QEvent::KeyPress || event_type == QEvent::KeyRelease ||
       event_type == QEvent::MouseButtonPress || event_type == QEvent::MouseButtonRelease ||
       event_type == QEvent::MouseButtonDblClick))
  {
    return true;
  }

  return QPushButton::event(event);
}

void MappingButton::mouseReleaseEvent(QMouseEvent* event)
{
  switch (event->button())
  {
  case Qt::MouseButton::LeftButton:
    if (m_reference->IsInput())
      QPushButton::mouseReleaseEvent(event);
    else
      emit AdvancedPressed();
    return;
  case Qt::MouseButton::MiddleButton:
    Clear();
    return;
  case Qt::MouseButton::RightButton:
    emit AdvancedPressed();
    return;
  default:
    return;
  }
}
