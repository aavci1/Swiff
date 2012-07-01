#ifndef SWIFFTYPES_H
#define SWIFFTYPES_H

#include <QPair>
#include <QString>

typedef quint8 u8;
typedef quint16 u16;
typedef quint32 u32;
typedef qint8 s8;
typedef qint16 s16;
typedef qint32 s32;

class f16 {
public:
  f16(const float v)
    : value(v) {
  }

  f16(const int v)
    : value(v) {
  }

  f16 operator = (const float v) {
    this->value = v;
    return *this;
  }

  operator float() {
    return this->value;
  }

private:
  float value;
};

class f32 {
public:
  f32(const float v)
    : value(v) {
  }

  f32(const int v)
    : value(v) {
  }

  f32 operator = (const float v) {
    this->value = v;
    return *this;
  }

  operator float() {
    return this->value;
  }

private:
  float value;
};

namespace Swiff {
  enum Tags {
    TagEnd = 0,
    TagShowFrame = 1,
    TagDefineShape = 2,
    TagPlaceObject = 4,
    TagRemoveObject = 5,
    TagDefineBits = 6,
    TagDefineButton = 7,
    TagJPEGTables = 8,
    TagSetBackgroundColor = 9,
    TagDefineFont = 10,
    TagDefineText = 11,
    TagDoAction = 12,
    TagDefineFontInfo = 13,
    TagDefineSound = 14,
    TagStartSound = 15,
    TagDefineButtonSound = 17,
    TagSoundStreamHead = 18,
    TagSoundStreamBlock = 19,
    TagDefineBitsLossless = 20,
    TagDefineBitsJPEG2 = 21,
    TagDefineShape2 = 22,
    TagDefineButtonCxform = 23,
    TagProtect = 24,
    TagPlaceObject2 = 26,
    TagRemoveObject2 = 28,
    TagDefineShape3 = 32,
    TagDefineText2 = 33,
    TagDefineButton2 = 34,
    TagDefineBitsJPEG3 = 35,
    TagDefineBitsLossless2 = 36,
    TagDefineEditText = 37,
    TagDefineSprite = 39,
    TagFrameLabel = 43,
    TagSoundStreamHead2 = 45,
    TagDefineMorphShape = 46,
    TagDefineFont2 = 48,
    TagExportAssets = 56,
    TagImportAssets = 57,
    TagEnableDebugger = 58,
    TagDoInitAction = 59,
    TagDefineVideoStream = 60,
    TagVideoFrame = 61,
    TagDefineFontInfo2 = 62,
    TagEnableDebugger2 = 64,
    TagScriptLimits = 65,
    TagSetTabIndex = 66,
    TagFileAttributes = 69,
    TagPlaceObject3 = 70,
    TagImportAssets2 = 71,
    TagDefineFontAlignZones = 73,
    TagCSMTextSettings = 74,
    TagDefineFont3 = 75,
    TagSymbolClass = 76,
    TagMetadata = 77,
    TagDefineScalingGrid = 78,
    TagDoABC = 82,
    TagDefineShape4 = 83,
    TagDefineMorphShape2 = 84,
    TagDefineSceneAndFrameLabelData = 86,
    TagDefineBinaryData = 87,
    TagDefineFontName = 88,
    TagStartSound2 = 89,
    TagDefineBitsJPEG4 = 90,
    TagDefineFont4 = 91
  };
  enum Actions {
    // SWF 3 Actions
    UnknownAction = 0x00,
    GotoFrame = 0x81,
    GetURL = 0x83,
    NextFrame = 0x04,
    PreviousFrame = 0x05,
    Play = 0x06,
    Stop = 0x07,
    ToggleQuality = 0x08,
    StopSounds = 0x09,
    WaitForFrame = 0x8A,
    SetTarget = 0x8B,
    GotoLabel = 0x8C,
    // SWF 4 Actions
    Push = 0x96,
    Pop = 0x17,
    Add = 0x0A,
    Subtract = 0x0B,
    Multiply = 0x0C,
    Divide = 0x0D,
    Equals = 0x0E,
    Less = 0x0F,
    And = 0x10,
    Or = 0x11,
    Not = 0x12,
    StringEquals = 0x13,
    StringLength = 0x14,
    StringAdd = 0x21,
    StringExtract = 0x15,
    StringLess = 0x29,
    MBStringLength = 0x31,
    MBStringExtract = 0x35,
    ToInteger = 0x18,
    CharToAscii = 0x32,
    AsciiToChar = 0x33,
    MBCharToAscii = 0x36,
    MBAsciiToChar = 0x37,
    Jump = 0x99,
    If = 0x9D,
    Call = 0x9E,
    GetVariable = 0x1C,
    SetVariable = 0x1D,
    GetURL2 = 0x9A,
    GotoFrame2 = 0x9F,
    SetTarget2 = 0x20,
    GetProperty = 0x22,
    SetProperty = 0x23,
    CloneSprite = 0x24,
    RemoveSprite = 0x25,
    StartDrag = 0x27,
    EndDrag = 0x28,
    WaitForFrame2 = 0x8D,
    Trace = 0x26,
    GetTime = 0x34,
    RandomNumber = 0x30,
    // SWF 5 Actions
    CallFunction = 0x3D,
    CallMethod = 0x52,
    ConstantPool = 0x88,
    DefineFunction = 0x9B,
    DefineLocal = 0x3C,
    DefineLocal2 = 0x41,
    Delete = 0x3A,
    Delete2 = 0x3B,
    Enumerate = 0x46,
    Equals2 = 0x49,
    GetMember = 0x4E,
    InitArray = 0x42,
    InitObject = 0x43,
    NewMethod = 0x53,
    NewObject = 0x40,
    SetMember = 0x4F,
    TargetPath = 0x45,
    With = 0x94,
    ToNumber = 0x4A,
    ToString = 0x4B,
    TypeOf = 0x44,
    Add2 = 0x47,
    Less2 = 0x48,
    Modulo = 0x3F,
    BitAnd = 0x60,
    BitLShift = 0x63,
    BitOr = 0x61,
    BitRShift = 0x64,
    BitURShift = 0x65,
    BitXor = 0x62,
    Decrement = 0x51,
    Increment = 0x50,
    PushDuplicate = 0x4C,
    Return = 0x3E,
    StackSwap = 0x4D,
    StoreRegister = 0x87,
    // SWF 6 Actions
    InstanceOf = 0x54,
    Enumerate2 = 0x55,
    StrictEquals = 0x66,
    Greater = 0x67,
    StringGreater = 0x68,
    // SWF 7 Actions
    DefineFunction2 = 0x8E,
    Extends = 0x69,
    CastOp = 0x2B,
    ImplementsOp = 0x2C,
    Try = 0x8F,
    Throw = 0x2A
  };

  // value types
  enum ValueType {
    String = 0x00,
    Float = 0x01,
    Null = 0x02,
    Undefined = 0x03,
    Register = 0x04,
    Boolean = 0x05,
    Double = 0x06,
    Integer = 0x07,
    Constant8 = 0x08,
    Constant16 = 0x09
  };
  typedef QPair<ValueType, QString> Variable;
  // fill style type
  enum FillStyle {
    SolidFill = 0x00,
    LinearGradientFill = 0x10,
    RadialGradientFill = 0x12,
    FocalGradientFill = 0x13,
    RepeatingBitmapFill = 0x40,
    ClippedBitmapFill = 0x41,
    NonSmoothedBitmapFill = 0x42,
    NonSmoothedClippedBitmapFill = 0x43
  };
  // button state
  enum ButtonState {
    Idle = 0x01,
    OverUp = 0x02,
    OverDown = 0x04,
    HitTest = 0x08,
    OutDown = 0x10
  };
  // button state transitions
  enum ButtonStateTransition {
    IdleToOverDown = 0x8000,
    OutDownToIdle = 0x4000,
    OutDownToOverDown = 0x2000,
    OverDownToOutDown = 0x1000,
    OverDownToOverUp = 0x0800,
    OverUpToOverDown = 0x0400,
    OverUpToIdle = 0x0200,
    IdleToOverUp = 0x0100,
    OverDownToIdle = 0x0001
  };
  // button filter type
  enum FilterType {
    DropShadow = 0,
    Blur = 1,
    Glow = 2,
    Bevel = 3,
    GradientGlow = 4,
    Convolution = 5,
    ColorMatrix = 6,
    GradientBevel = 7,
    Unknown = 255
  };
}

#endif
