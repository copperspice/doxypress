/************************************************************************
*
* Copyright (c) 2014-2022 Barbara Geller & Ansel Sermersheim
*
* DoxyPress is free software: you can redistribute it and/or
* modify it under the terms of the GNU General Public License version 2
* as published by the Free Software Foundation.
*
* DoxyPress is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*
* Documents produced by DoxyPress are derivative works derived from the
* input used in their production; they are not affected by this license.
*
*************************************************************************/

#ifndef BUCKET_H
#define BUCKET_H

#include <QString>

class QTextCodec;

class test_Bucket
{
 public:
   static QString escape(const QString &plain);

   CORE_CS_ENUM(BGMode)
   CORE_CS_ENUM(CursorShape)
   CORE_CS_ENUM(DateFormat)
   CORE_CS_ENUM(DayOfWeek)
   CORE_CS_ENUM(LayoutDirection)
   CORE_CS_ENUM(PenStyle)
   CORE_CS_ENUM(SizeHint)
   CORE_CS_ENUM(TextFormat)
   CORE_CS_ENUM(TextElideMode)
   CORE_CS_ENUM(TimeSpec)

   CORE_CS_ENUM(Orientation)
   CORE_CS_FLAG(Orientation, Orientations)

   CORE_CS_ENUM(AlignmentFlag)
   CORE_CS_FLAG(AlignmentFlag, Alignment)

   CORE_CS_ENUM(WindowType)
   CORE_CS_FLAG(WindowType, WindowFlags)

   CORE_CS_ENUM(WindowState)
   CORE_CS_FLAG(WindowState, WindowStates)

   CORE_CS_ENUM(GestureState)
   CORE_CS_ENUM(GestureType)

   enum BGMode {
      TransparentMode,
      OpaqueMode
   };

   enum GlobalColor {
      color0,
      color1,
      black,
      white,
      darkGray,
      gray,
      lightGray,
      red,
      green,
      blue,
      cyan,
      magenta,
      yellow,
      darkRed,
      darkGreen,
      darkBlue,
      darkCyan,
      darkMagenta,
      darkYellow,
      transparent
   };

   enum KeyboardModifier {
      NoModifier           = 0x00000000,
      ShiftModifier        = 0x02000000,
      ControlModifier      = 0x04000000,
      AltModifier          = 0x08000000,
      MetaModifier         = 0x10000000,
      KeypadModifier       = 0x20000000,
      GroupSwitchModifier  = 0x40000000,
      KeyboardModifierMask = 0xfe000000
   };
   using KeyboardModifiers = QFlags<KeyboardModifier>;

   enum Modifier {
      META          = Qt::MetaModifier,
      SHIFT         = Qt::ShiftModifier,
      CTRL          = Qt::ControlModifier,
      ALT           = Qt::AltModifier,
      MODIFIER_MASK = KeyboardModifierMask,
      UNICODE_ACCEL = 0x00000000
   };

   enum Orientation {
      Horizontal = 0x1,
      Vertical = 0x2
   };

   using Orientations = QFlags<Orientation>;

   enum SortOrder {
      AscendingOrder,
      DescendingOrder
   };

   enum AlignmentFlag {
      AlignLeft = 0x0001,
      AlignLeading = AlignLeft,
      AlignRight = 0x0002,
      AlignTrailing = AlignRight,
      AlignHCenter = 0x0004,
      AlignJustify = 0x0008,
      AlignAbsolute = 0x0010,
      AlignHorizontal_Mask = AlignLeft | AlignRight | AlignHCenter | AlignJustify | AlignAbsolute,

      AlignTop = 0x0020,
      AlignBottom = 0x0040,
      AlignVCenter = 0x0080,
      AlignVertical_Mask = AlignTop | AlignBottom | AlignVCenter,

      AlignCenter = AlignVCenter | AlignHCenter
   };

   using Alignment = QFlags<AlignmentFlag>;

   enum TextFlag {
      TextSingleLine = 0x0100,
      TextDontClip = 0x0200,
      TextExpandTabs = 0x0400,
      TextShowMnemonic = 0x0800,
      TextWordWrap = 0x1000,
      TextWrapAnywhere = 0x2000,
      TextDontPrint = 0x4000,
      TextIncludeTrailingSpaces = 0x08000000,
      TextHideMnemonic = 0x8000,
      TextJustificationForced = 0x10000,
      TextForceLeftToRight = 0x20000,
      TextForceRightToLeft = 0x40000,
      TextLongestVariant = 0x80000,
      TextBypassShaping = 0x100000
   };

   enum WindowType {
      Widget = 0x00000000,
      Window = 0x00000001,
      Dialog = 0x00000002 | Window,
      Sheet = 0x00000004 | Window,
      Drawer = 0x00000006 | Window,
      Popup = 0x00000008 | Window,
      Tool = 0x0000000a | Window,
      ToolTip = 0x0000000c | Window,
      SplashScreen = 0x0000000e | Window,
      Desktop = 0x00000010 | Window,
      SubWindow =  0x00000012
   };

   using WindowFlags = QFlags<WindowType>;

   enum WindowState {
      WindowNoState    = 0x00000000,
      WindowMinimized  = 0x00000001,
      WindowMaximized  = 0x00000002,
      WindowFullScreen = 0x00000004,
      WindowActive     = 0x00000008
   };

   using WindowStates = QFlags<WindowState>;

   enum ArrowType {
      NoArrow,
      UpArrow,
      DownArrow,
      LeftArrow,
      RightArrow
   };

   enum PenStyle {
      NoPen,
      SolidLine,
      DashLine,
      DotLine,
      DashDotLine,
      DashDotDotLine,
      CustomDashLine,
      MPenStyle = 0x0f
   };

   enum DateFormat {
      TextDate,                     // default
      ISODate,                      // ISO 8601
      SystemLocaleDate,             // deprecated
      LocalDate = SystemLocaleDate, // deprecated
      LocaleDate,                   // deprecated
      SystemLocaleShortDate,
      SystemLocaleLongDate,
      DefaultLocaleShortDate,
      DefaultLocaleLongDate
   };

   enum TimeSpec {
      LocalTime,
      UTC,
      OffsetFromUTC
   };

   enum DayOfWeek {
      Monday    = 1,
      Tuesday   = 2,
      Wednesday = 3,
      Thursday  = 4,
      Friday    = 5,
      Saturday  = 6,
      Sunday    = 7
   };

   enum LayoutDirection {
      LeftToRight,
      RightToLeft,
      LayoutDirectionAuto
   };

#if defined(Q_OS_DARWIN)
   typedef void *HANDLE;

#elif defined(Q_OS_WIN)
   typedef void *HANDLE;

#elif defined(Q_OS_UNIX)
   typedef unsigned long HANDLE;

#else
   typedef void *HANDLE;

#endif

   typedef WindowFlags WFlags;

   enum SizeHint {
      MinimumSize,
      PreferredSize,
      MaximumSize,
      MinimumDescent,
      NSizeHints
   };

   enum HitTestAccuracy { ExactHit, FuzzyHit };

   enum WhiteSpaceMode {
      WhiteSpaceNormal,
      WhiteSpacePre,
      WhiteSpaceNoWrap,
      WhiteSpaceModeUndefined = -1
   };

   static bool mightBeRichText(const QString &);
   static QString convertFromPlainText(const QString &plain, WhiteSpaceMode mode = WhiteSpacePre);
   static QTextCodec *codecForHtml(const QByteArray &ba);
};

#endif
