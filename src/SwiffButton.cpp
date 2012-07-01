#include "SwiffButton.h"

#include "SwiffAction.h"
#include "SwiffActionScriptEngine.h"
#include "SwiffBitStream.h"
#include "SwiffCxForm.h"
#include "SwiffDocument.h"
#include "SwiffFilter.h"
#include "SwiffShape.h"
#include "SwiffSprite.h"

#include <QMap>
#include <QMatrix>
#include <QString>

namespace Swiff {

  class ButtonRecord {
  public:
    ButtonRecord() : mItem(0), mColorTransform(true) {
    }

    ~ButtonRecord() {
      qDeleteAll(mFilterList);
    }

    Item *mItem;
    u8 mFlags;
    u16 mCharacterId;
    u16 mPlaceDepth;
    QMatrix mMatrix;
    CxForm mColorTransform;
    QVector<Filter *> mFilterList;
    u8 mBlendMode;
  };

  class ButtonAction {
  public:
    ButtonAction(): mKeyCode(0) {}

    const QVector<Action *> &actions() const {
      return mActions;
    }
    /** Sets actions. */
    void setActions(const QVector<Action *> actions) {
      mActions = actions;
    }
    /** \returns conditions to trigger this action. */
    u16 conditions() const {
      return mConditions;
    }
    /** \returns key code to trigger this action. */
    u8 keyCode() const {
      return mKeyCode;
    }
    /** Sets conditions and key code. */
    void setFlags(u16 flags) {
      mKeyCode = (flags & 0x00fe) >> 1;
      mConditions = flags & 0xff01;
    }

    void parse(BitStream &bitstream) {
      bitstream.align();
      u16 flags = 0;
      bitstream >> flags;
      // TODO: handle assigned keys
      setFlags(flags);
      setActions(ActionScriptEngine::parseActions(bitstream));
    }
  private:
    /** Conditions to execute the actions. */
    u16 mConditions;
    /** Key code to execute the actions. */
    u8 mKeyCode;
    /** Actions to execute. */
    QVector<Action *> mActions;
  };

  ButtonRecord *parseButtonRecord(BitStream &bitstream, int version) {
    ButtonRecord *record = new ButtonRecord();
    bitstream >> record->mFlags >> record->mCharacterId >> record->mPlaceDepth >> record->mMatrix;
    if (version >= 2) {
      bitstream >> record->mColorTransform;
      if (record->mFlags & 0x10) {
        u8 filterCount = 0;
        bitstream >> filterCount;
        for (u8 i = 0; i < filterCount; ++i) {
          Filter *filter = Filter::fromFilterId(bitstream.currentu8());
          filter->parse(bitstream);
          record->mFilterList.append(filter);
        }
      }
      if (record->mFlags & 0x20)
        bitstream >> record->mBlendMode;
    }
    return record;
  }

  class ButtonPrivate {
  public:
    ButtonPrivate(Sprite *parent) : mParent(parent), mState(Swiff::Idle) {
    }

    ~ButtonPrivate() {
      qDeleteAll(mButtonRecords);
    }

    Sprite *mParent;
    QMap<u16, ButtonRecord *> mButtonRecords;
    QVector<ButtonAction *> mButtonActions;
    ButtonState mState;
    QPolygonF mHitArea;
    u8 mFlags;
  };

  Button::Button(Sprite *parent) : Item(Swiff::ItemButton), d(new ButtonPrivate(parent)) {
  }

  Button::~Button() {
    delete d;
  }

  void Button::parse(BitStream &bitstream, int version) {
    bitstream >> mCharacterId;
    u16 actionOffset = 0;
    u32 actionStart = 0;
    if (version == 2) {
      bitstream >> d->mFlags;
      bitstream >> actionOffset;
      actionStart = bitstream.byteIndex() + actionOffset - 2;
    }
    // parse button record
    while (bitstream.currentu8()) {
      ButtonRecord *record = parseButtonRecord(bitstream, version);
      // get record item
      if (d->mParent->document()->dictionary().contains(record->mCharacterId)) {
        record->mItem = d->mParent->document()->dictionary().value(record->mCharacterId);
        // calculate hit area
        if ((record->mFlags & Swiff::HitTest) && (record->mItem->mItemType == Swiff::ItemShape))
          d->mHitArea << static_cast<Shape *>(record->mItem)->combined().toFillPolygon(record->mMatrix);
      }
      d->mButtonRecords.insertMulti(record->mPlaceDepth, record);
      // align to the next byte to be able to correctly check the end flag
      bitstream.align();
    }
    // skip character end flag
    bitstream.skip(1, 0);
    if (version == 1) {
      // parse button actions
      ButtonAction *buttonAction = new ButtonAction();
      buttonAction->setFlags(Swiff::OverDownToOverUp);
      buttonAction->setActions(ActionScriptEngine::parseActions(bitstream));
      d->mButtonActions.append(buttonAction);
    } else if (version == 2) {
      // parse button actions
      while (actionOffset) {
        bitstream.seek(actionStart);
        // calculate start of the next action
        bitstream >> actionOffset;
        actionStart = bitstream.byteIndex() + actionOffset - 2;
        // parse the
        ButtonAction *buttonAction = new ButtonAction();
        buttonAction->parse(bitstream);
        d->mButtonActions.append(buttonAction);
      }
    }
  }

  void Button::paint(QPainter &painter) {
    foreach(const ButtonRecord * record, d->mButtonRecords) {
      if ((record->mFlags & d->mState) && record->mItem) {
        painter.save();
        painter.setMatrix(record->mMatrix, true);
        record->mItem->paint(painter);
        painter.restore();
      }
    }
  }

  QRect Button::bounds() const {
    QRect bounds;
    // start/stop sprites
    foreach (const ButtonRecord *state, d->mButtonRecords) {
      // skip, if current state exists in both old and new states
      if ((state->mFlags & d->mState) == 0)
        continue;
      // skip, if current state does not contain a sprite
      if (state->mItem != 0 && state->mItem->mItemType == Swiff::ItemShape) {
        Shape *shape = static_cast<Shape *>(state->mItem);
        bounds = bounds.unite(shape->bounds());
      } else if (state->mItem != 0 && state->mItem->mItemType == Swiff::ItemSprite) {
        // TODO: add sprites to bounds calculation too
        // Sprite *sprite = static_cast<Sprite *>(state->mItem);
        // bounds.unite(sprite->bounds());
      }
    }
    return bounds;
  }

  QVector<QPair<QString, QString> > Button::properties() const {
    QVector<QPair<QString, QString> > p;

    p.append(QPair<QString, QString>(QString("Width"), QString::number(int(bounds().width() * 0.05f))));
    p.append(QPair<QString, QString>(QString("Height"), QString::number(int(bounds().height() * 0.05f))));

    return p;
  }

  ButtonState Button::state() const {
    return d->mState;
  }

  void Button::setState(ButtonState state) {
    d->mState = state;
  }

  void Button::processTransition(ButtonStateTransition transition) {
    for (int i = 0; i < d->mButtonActions.count(); ++i)
      if (d->mButtonActions.at(i)->conditions() & transition)
        ActionScriptEngine::executeActions(d->mParent, d->mButtonActions.at(i)->actions());
    // cache old state
    ButtonState oldState = d->mState;
    // set the visual state
    switch (transition) {
    case Swiff::OverUpToOverDown:
    case Swiff::OutDownToOverDown:
    case Swiff::IdleToOverDown:
      d->mState = Swiff::OverDown;
      break;
    case Swiff::IdleToOverUp:
    case Swiff::OverDownToOverUp:
    case Swiff::OverDownToOutDown:
      d->mState = Swiff::OverUp;
      break;
    case Swiff::OverUpToIdle:
    case Swiff::OutDownToIdle:
    case Swiff::OverDownToIdle:
      d->mState = Swiff::Idle;
      break;
    }
    // start/stop sprites
    foreach(const ButtonRecord * state, d->mButtonRecords) {
      // skip, if current state exists in both old and new states
      if ((state->mFlags & oldState) && (state->mFlags & d->mState))
        continue;
      // skip, if current state does not contain a sprite
      if (state->mItem == 0 || state->mItem->mItemType != Swiff::ItemSprite)
        continue;
      Sprite *sprite = static_cast<Sprite *>(state->mItem);
      // stop sprites from the old state
      if (state->mFlags & oldState)
        sprite->stop();
      // start sprites from the new state
      if (state->mFlags & d->mState)
        sprite->play();
    }
  }

  bool Button::contains(const QMatrix &matrix, const QPointF &position) const {
    return matrix.map(d->mHitArea).containsPoint(position, Qt::WindingFill);
  }

  Button *Button::clone() {
    Button *button = new Button();
    // copy properties
    button->mCharacterId = this->mCharacterId;
    button->mItemType = this->mItemType;
    button->d->mButtonActions = this->d->mButtonActions;
    button->d->mFlags = this->d->mFlags;
    button->d->mHitArea = this->d->mHitArea;
    button->d->mParent = this->d->mParent;
    button->d->mState = this->d->mState;
    button->d->mButtonRecords = this->d->mButtonRecords;
    // return cloned object
    return button;
  }
}
