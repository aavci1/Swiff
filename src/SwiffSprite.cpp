#include "SwiffSprite.h"

#include "SwiffActionScriptEngine.h"
#include "SwiffBitmap.h"
#include "SwiffBitStream.h"
#include "SwiffButton.h"
#include "SwiffDoABC.h"
#include "SwiffDocument.h"
#include "SwiffFont.h"
#include "SwiffFrame.h"
#include "SwiffObject.h"
#include "SwiffPlaceObject.h"
#include "SwiffPlaceObject2.h"
#include "SwiffPlaceObject3.h"
#include "SwiffRemoveObject.h"
#include "SwiffRemoveObject2.h"
#include "SwiffSetBackgroundColor.h"
#include "SwiffShape.h"
#include "SwiffSound.h"
#include "SwiffVideo.h"
#include "SwiffTag.h"
#include "SwiffText.h"

#include <QDebug>
#include <QMap>
#include <QString>
#include <QStack>
#include <QTimer>
#include <QVector>
#include <QPainter>

namespace Swiff {
  class SpritePrivate {
  public:
    SpritePrivate(Document *document) : mDocument(document), mCurrentFrame(-1),
    mState(Swiff::SpriteStopped), mLoop(true), mMouseDown(false) {
    }

    ~SpritePrivate() {
      qDeleteAll(mDisplayList);
      qDeleteAll(mFrames);
      qDeleteAll(mStreams);
    }

    QBrush mBackgroundBrush;
    Document *mDocument;
    u32 mCurrentFrame;
    QMap<u16, Object *> mDisplayList;
    QVector<Frame *> mFrames;
    QVector<Sound *> mStreams;
    SpriteState mState;
    QString mName;
    bool mLoop;
    bool mMouseDown;
    QTimer mFrameTimer;
  };

  Sprite::Sprite(Document *document): Item(Swiff::ItemSprite), d(new SpritePrivate(document)) {
    // create the frame timer
    QObject::connect(&d->mFrameTimer, SIGNAL(timeout()), this, SLOT(nextFrame()));
    d->mFrameTimer.setInterval((int)(1000.0f / d->mDocument->frameRate()));
  }

  Sprite::~Sprite() {
    delete d;
  }

  u32 consumeHeader(BitStream &bitstream) {
    u16 header = 0;
    bitstream >> header;
    u32 length = header & 0x3f;
    if (length >= 0x3f)
      bitstream >> length;
    // return the next tag position
    return bitstream.byteIndex() + length;
  }

  void Sprite::parse(BitStream &bitstream, int /*version*/) {
    // read and discard
    u16 frameCount;
    bitstream >> frameCount;
    // current frame's tag list
    QVector<Tag *> tags;
    QVector<Action *> actions;
    // streaming sound
    Sound *stream = 0;
    do {
      u32 nextTag = 0;
      // parse tag id
      u16 tagId = bitstream.currentu16() >> 6;
      if (tagId == 0)
        break;
      switch (tagId) {
      case TagJPEGTables: {
          // consume record header
          nextTag = consumeHeader(bitstream);
          u32 size = nextTag - bitstream.byteIndex();
          if (size == 0)
            break;
          u8 *jpegTables = new u8[size];
          bitstream.readBytes(jpegTables, size);
          // clean erroneous markers
          removeWrongMarker(jpegTables, size);
          // save jpeg tables skipping the JPEG EOI marker if present
          if (size >= 2 && jpegTables[size - 2] == 0xFF && jpegTables[size - 1] == 0xD9)
            document()->setJPEGTables(jpegTables, size - 2);
          else
            document()->setJPEGTables(jpegTables, size);
          // clean up
          delete jpegTables;
        }
        break;
      case TagDefineBits: {
          // consume record header
          nextTag = consumeHeader(bitstream);
          u16 characterId = 0;
          bitstream >> characterId;
          u32 size = nextTag - bitstream.byteIndex();
          u8 *bits = new u8[size];
          bitstream.readBytes(bits, size);
          // clean erroneous markers
          removeWrongMarker(bits, size);
          // construct the JPEG data
          QByteArray bytes;
          bytes.append((const char *)(document()->JPEGTables()), document()->JPEGTablesSize());
          // skip the jpeg SOI marker at the beginning if present
          if (document()->JPEGTablesSize() != 0 && bits[0] == 0xFF && bits[1] == 0xD8)
            bytes.append((const char *)(bits + 2), size - 2);
          else
            bytes.append((const char *)(bits), size);
          Bitmap *item = new Bitmap();
          item->mCharacterId = characterId;
          item->mImage.loadFromData(bytes);
          // add item to the dictionary
          d->mDocument->dictionary()[item->mCharacterId] = item;
          // clean up
          delete bits;
        }
        break;
      case TagDefineBitsJPEG2:
      case TagDefineBitsJPEG3:
      case TagDefineBitsJPEG4: {
          // consume record header
          nextTag = consumeHeader(bitstream);
          // create and parse item
          Bitmap *item = 0;
          // parse the bitmap item
          if (tagId == TagDefineBitsJPEG2)
            item = parseBitsJPEG(bitstream, 2, bitstream.byteIndex(), nextTag - bitstream.byteIndex());
          else if (tagId == TagDefineBitsJPEG3)
            item = parseBitsJPEG(bitstream, 3, bitstream.byteIndex(), nextTag - bitstream.byteIndex());
          else if (tagId == TagDefineBitsJPEG4)
            item = parseBitsJPEG(bitstream, 4, bitstream.byteIndex(), nextTag - bitstream.byteIndex());
          // add item to the dictionary
          d->mDocument->dictionary()[item->mCharacterId] = item;
        }
        break;
      case TagDefineBitsLossless:
      case TagDefineBitsLossless2: {
          // consume record header
          nextTag = consumeHeader(bitstream);
          // create and parse item
          Bitmap *item = 0;
          // parse the bitmap item
          if (tagId == TagDefineBitsLossless)
            item = parseBitsLossless(bitstream, 1, nextTag - bitstream.byteIndex());
          else if (tagId == TagDefineBitsLossless2)
            item = parseBitsLossless(bitstream, 2, nextTag - bitstream.byteIndex());
          // add item to the dictionary
          d->mDocument->dictionary()[item->mCharacterId] = item;
        }
        break;
      case TagDefineButton:
      case TagDefineButton2: {
          // consume record header
          nextTag = consumeHeader(bitstream);
          // create and parse item
          Button *item = new Button(this);
          // parse the button item
          if (tagId == TagDefineButton)
            item->parse(bitstream, 1);
          else if (tagId == TagDefineButton2)
            item->parse(bitstream, 2);
          // add item to the dictionary
          d->mDocument->dictionary()[item->mCharacterId] = item;
        }
        break;
      case TagDefineFont:
      case TagDefineFont2:
      case TagDefineFont3:
      case TagDefineFont4: {
          // consume record header
          nextTag = consumeHeader(bitstream);
          // create and parse item
          Font *item = new Font(this);
          // parse the shape item
          if (tagId == TagDefineFont)
            item->parse(bitstream, 1);
          else if (tagId == TagDefineFont2)
            item->parse(bitstream, 2);
          else if (tagId == TagDefineFont3)
            item->parse(bitstream, 3);
          else if (tagId == TagDefineFont4)
            item->parse(bitstream, 4);
          // add item to the dictionary
          d->mDocument->dictionary()[item->mCharacterId] = item;
        }
        break;
      case TagDefineFontInfo:
      case TagDefineFontInfo2: {
          // consume record header
          nextTag = consumeHeader(bitstream);
          // read font id
          u16 fontId = 0;
          bitstream >> fontId;
          // check if the font exists
          if (!d->mDocument->dictionary().contains(fontId))
            break;
          Font *font = static_cast<Font *>(d->mDocument->dictionary()[fontId]);
          if (tagId == TagDefineFontInfo)
            font->parseInfo(bitstream, 1);
          else if (tagId == TagDefineFontInfo2)
            font->parseInfo(bitstream, 2);
        }
        break;
      case TagDefineFontName: {
          // consume record header
          nextTag = consumeHeader(bitstream);
          // read font id
          u16 fontId = 0;
          bitstream >> fontId;
          // check if the font exists
          if (!d->mDocument->dictionary().contains(fontId))
            break;
          Font *font = static_cast<Font *>(d->mDocument->dictionary()[fontId]);
          // read name info
          QString name;
          bitstream >> name;
          font->setName(name);
          // read copyright info
          QString copyright;
          bitstream >> copyright;
          font->setCopyright(copyright);
        }
        break;
      case TagDefineMorphShape: {
          qDebug() << "TODO: Implement DefineMorphShape";
          // consume record header
          nextTag = consumeHeader(bitstream);
      }
      case TagDefineMorphShape2: {
          qDebug() << "TODO: Implement DefineMorphShape2";
          // consume record header
          nextTag = consumeHeader(bitstream);
        }
        break;
      case TagDefineShape:
      case TagDefineShape2:
      case TagDefineShape3:
      case TagDefineShape4: {
          // consume record header
          nextTag = consumeHeader(bitstream);
          // create and parse item
          Shape *item = new Shape(this);
          // parse the shape item
          if (tagId == TagDefineShape)
            item->parse(bitstream, 1);
          else if (tagId == TagDefineShape2)
            item->parse(bitstream, 2);
          else if (tagId == TagDefineShape3)
            item->parse(bitstream, 3);
          else if (tagId == TagDefineShape4)
            item->parse(bitstream, 4);
          // add item to the dictionary
          d->mDocument->dictionary()[item->mCharacterId] = item;
        }
        break;
      case TagDefineSound: {
          // consume record header
          nextTag = consumeHeader(bitstream);
          // create and parse item
          Sound *sound = new Sound();
          // read character id
          bitstream >> sound->mCharacterId;
          // parse the sprite item
          sound->parse(bitstream, 1);
          // parse sound data
          u32 size = nextTag - bitstream.byteIndex();
          u8 *data = new u8[size];
          bitstream.readBytes(data, size);
          sound->setData(data, size);
          // add item to the dictionary
          d->mDocument->dictionary()[sound->mCharacterId] = sound;
        }
        break;
      case TagSoundStreamHead:
      case TagSoundStreamHead2: {
          // consume record header
          nextTag = consumeHeader(bitstream);
          // skip playback recommendation flags
          bitstream.skip(1);
          // create and parse sound item
          stream = new Sound();
          stream->parse(bitstream, 1);
          // emit warning if sound format is not implemented
          if (stream->format() == 1)
            qDebug() << "WARN: ADPCM codec not implemented!";
          if (stream->format() == 3)
            qDebug() << "WARN: Uncompressed, little-endian codec not implemented!";
          if (stream->format() == 4 || stream->format() == 5 || stream->format() == 6)
            qDebug() << "WARN: Nellymoser codec not implemented!";
          if (stream->format() == 11)
            qDebug() << "WARN: Speex codec not implemented!";
          if (stream->format() != 0 && stream->format() != 2) {
            delete stream;
            stream = 0;
            break;
          }
          // add stream to the streams list
          d->mStreams.append(stream);
          // skip latency seek
        }
        break;
      case TagSoundStreamBlock: {
          // consume record header
          nextTag = consumeHeader(bitstream);
          if (stream == 0)
            break;
          u32 size = nextTag - bitstream.byteIndex();
          u8 *data = new u8[size];
          // read new data
          bitstream.readBytes(data, size);
          // update stream data
          if (stream->format() == 0)
            stream->appendData(data, size);
          else if (stream->format() == 2)
            stream->appendData(data + 4, size - 4);
          // clean up
          delete data;
        }
        break;
      case TagDefineSprite: {
          // consume record header
          nextTag = consumeHeader(bitstream);
          // create and parse item
          Sprite *item = new Sprite(d->mDocument);
          // read character id
          bitstream >> item->mCharacterId;
          // parse the sprite item
          item->parse(bitstream, 1);
          // add item to the dictionary
          d->mDocument->dictionary()[item->mCharacterId] = item;
        }
        break;
      case TagDefineText:
      case TagDefineText2: {
          // consume record header
          nextTag = consumeHeader(bitstream);
          // create and parse item
          Text *item = new Text(this);
          // parse the text item
          if (tagId == TagDefineText)
            item->parse(bitstream, 1);
          else if (tagId == TagDefineText2)
            item->parse(bitstream, 2);
          // add item to the dictionary
          d->mDocument->dictionary()[item->mCharacterId] = item;
        }
        break;
      case TagDefineEditText: {
          // consume record header
          nextTag = consumeHeader(bitstream);
          qDebug() << "TODO: Implement DefineEditText";
        }
        break;
      case TagDefineVideoStream: {
          // consume record header
          nextTag = consumeHeader(bitstream);
          // create and parse item
          Video *video = new Video();
          // read character id
          bitstream >> video->mCharacterId;
          // parse the sprite item
          video->parse(bitstream, 1);
          // add item to the dictionary
          d->mDocument->dictionary()[video->mCharacterId] = video;
        }
        break;
      case TagVideoFrame: {
          // consume record header
          nextTag = consumeHeader(bitstream);
          u16 streamId;
          bitstream >> streamId;
          if (!d->mDocument->dictionary().contains(streamId))
            break;
          Video *video = reinterpret_cast<Video *>(d->mDocument->dictionary()[streamId]);
          if (!video)
            break;
          // skip frame num
          bitstream.skip(2);
          // read data
          u32 size = nextTag - bitstream.byteIndex();
          u8 *data = new u8[size];
          // read new data
          bitstream.readBytes(data, size);
          // update video data
          video->appendData(data, size);
        }
        break;
      case TagDoAction: {
          // consume record header
          nextTag = consumeHeader(bitstream);
          // parse actions
          actions = ActionScriptEngine::parseActions(bitstream);
        }
        break;
      case TagSetBackgroundColor: {
          Tag *tag = new SetBackgroundColor();
          tag->parse(bitstream);
          // calculate next tag position
          nextTag = tag->index() + tag->length();
        }
        break;
      case TagPlaceObject: {
          Tag *tag = new PlaceObject();
          tag->parse(bitstream);
          tags.append(tag);
          // calculate next tag position
          nextTag = tag->index() + tag->length();
        }
        break;
      case TagPlaceObject2: {
          Tag *tag = new PlaceObject2();
          tag->parse(bitstream);
          tags.append(tag);
          // calculate next tag position
          nextTag = tag->index() + tag->length();
        }
        break;
      case TagPlaceObject3: {
          Tag *tag = new PlaceObject3();
          tag->parse(bitstream);
          tags.append(tag);
          // calculate next tag position
          nextTag = tag->index() + tag->length();
        }
        break;
      case TagRemoveObject: {
          Tag *tag = new RemoveObject();
          tag->parse(bitstream);
          tags.append(tag);
          // calculate next tag position
          nextTag = tag->index() + tag->length();
        }
        break;
      case TagRemoveObject2: {
          Tag *tag = new RemoveObject2();
          tag->parse(bitstream);
          tags.append(tag);
          // calculate next tag position
          nextTag = tag->index() + tag->length();
        }
        break;
      case TagShowFrame: {
          // consume record header
          nextTag = consumeHeader(bitstream);
          // create a new frame
          Frame *frame = new Frame(this);
          // assign frame tags
          foreach (Tag *tag, tags)
            frame->tags().append(tag);
          // assign frame actions
          frame->setActions(actions);
          // append frame to the list
          d->mFrames.append(frame);
          // clear tag list
          tags.clear();
        }
        break;
      default:
        nextTag = consumeHeader(bitstream);
        break;
      }
      bitstream.seek(nextTag);
    } while (1);
  }

  void Sprite::paint(QPainter &painter) {
    // return in case of an invalid frame number
    if (d->mCurrentFrame >= (unsigned int)(d->mFrames.count())) {
      return;
    }
    // do the painting
    foreach(Object * object, d->mDisplayList) {
      if (object == 0) {
        continue;
      }
      QMatrix oldMatrix = painter.matrix();
      painter.setMatrix(object->matrix(), true);
      // TODO: apply color transform
      qreal oldOpacity = painter.opacity();
      painter.setOpacity(oldOpacity * object->colorTransform().mAlphaMultTerm / 256.0
                         + object->colorTransform().mAlphaAddTerm);
      object->item()->paint(painter);
      painter.setOpacity(oldOpacity);
      painter.setMatrix(oldMatrix);
    }
  }

  QRect Sprite::bounds() const {
    // TODO: calculate bounds of the sprite
    return QRect();
  }

  QVector<QPair<QString, QString> > Sprite::properties() const {
    QVector<QPair<QString, QString> > p;

    p.append(QPair<QString, QString>(QString("Width"), QString::number(int(bounds().width() * 0.05f))));
    p.append(QPair<QString, QString>(QString("Height"), QString::number(int(bounds().height() * 0.05f))));
    p.append(QPair<QString, QString>(QString("Frame Rate"), QString::number(int(1000.0f / d->mDocument->frameRate()))));
    p.append(QPair<QString, QString>(QString("Frame Count"), QString::number(d->mFrames.size())));

    return p;
  }

  QVector<Frame *> Sprite::frames() const {
    return d->mFrames;
  }

  QVector<Sound *> Sprite::streams() const {
    return d->mStreams;
  }

  Document *Sprite::document() const {
    return d->mDocument;
  }

  QBrush Sprite::backgroundBrush() const {
    return d->mBackgroundBrush;
  }

  void Sprite::prevFrame() {
    if (d->mState != Swiff::SpritePlaying) {
      return;
    }
    if (d->mCurrentFrame <= 0) {
      return;
    }
    gotoFrame(d->mCurrentFrame - 1);
  }

  void Sprite::nextFrame() {
    if (d->mState != Swiff::SpritePlaying) {
      return;
    }
    gotoFrame(d->mCurrentFrame + 1);
  }

  void Sprite::gotoFramePtr(Frame *frame) {
    gotoFrame(d->mFrames.indexOf(frame));
  }

  void Sprite::gotoFrame(u16 frameNo) {
    // TODO: handle child sprites too
    if (frameNo >= d->mFrames.count()) {
      if (d->mFrames.count() <= 1) {
        return;
      }
      if (!d->mLoop) {
        return;
      }
      // return to the beginning
      frameNo = 0;
    }
    if (frameNo == d->mCurrentFrame) {
      return;
    }
    if (frameNo < d->mCurrentFrame) {
      // delete all objects
      qDeleteAll(d->mDisplayList);
      // clear everything
      d->mDisplayList.clear();
      // fast forward all tags
      for (u16 i = 0; i <= frameNo; ++i) {
        processTags(d->mFrames.at(i)->tags());
      }
    } else {
      // fast forward all tags
      for (u16 i = d->mCurrentFrame + 1; i <= frameNo; ++i) {
        processTags(d->mFrames.at(i)->tags());
      }
    }
    // set current frame
    d->mCurrentFrame = frameNo;
    // process actions
    ActionScriptEngine::executeActions(this, d->mFrames.at(d->mCurrentFrame)->actions());
    // emit update signal
    emit update();
  }

  void Sprite::gotoFrame(const QString &frameLabel) {
    for (int i = 0; i < d->mFrames.count(); ++i) {
      if (d->mFrames.at(i)->label().compare(frameLabel)) {
        gotoFrame(i);
        break;
      }
    }
  }

  SpriteState Sprite::state() const {
    return d->mState;
  }

  void Sprite::setState(SpriteState state) {
    d->mState = state;
  }

  void Sprite::play() {
    if (d->mState == Swiff::SpritePlaying) {
      return;
    }
    if (d->mState == Swiff::SpriteStopped) {
      d->mCurrentFrame = -1;
    }
    // start the timer
    d->mFrameTimer.start();
    // update the state
    d->mState = Swiff::SpritePlaying;
  }

  void Sprite::pause() {
    // stop the timer
    d->mFrameTimer.stop();
    // update the state
    d->mState = Swiff::SpritePaused;
  }

  void Sprite::stop() {
    // stop the timer
    d->mFrameTimer.stop();
    // update the state
    d->mState = Swiff::SpriteStopped;
  }

  void Sprite::rewind() {
    // move to the beginning
    d->mCurrentFrame = 0;
  }

  void Sprite::mouseMoved(const QPointF &position, bool &overButton, bool &sceneChanged) {
    // TODO: mouse tracking
    // TODO: menu buttons
    overButton = false;
    sceneChanged = false;
    foreach(Object * object, d->mDisplayList) {
      if (object != 0 && object->item() != 0 && object->item()->mItemType == Swiff::ItemButton) {
        Button *button = static_cast<Button *>(object->item());
        if (!overButton && button->contains(object->matrix(), position)) {
          if (d->mMouseDown) {
            if (button->state() != Swiff::OverDown) {
              button->processTransition(Swiff::OutDownToOverDown);
              sceneChanged = true;
            }
          } else {
            if (button->state() == Swiff::Idle) {
              button->processTransition(Swiff::IdleToOverUp);
              sceneChanged = true;
            }
          }
        } else {
          if (d->mMouseDown) {
            if (button->state() == Swiff::OverDown) {
              button->processTransition(Swiff::OverDownToOutDown);
              sceneChanged = true;
            }
          } else {
            if (button->state() == Swiff::OverUp) {
              button->processTransition(Swiff::OverUpToIdle);
              sceneChanged = true;
            }
          }
        }
        if (button->state() == Swiff::OverUp) {
          overButton = true;
        }
      }
    }
  }

  void Sprite::mousePressed(const QPointF &position, bool &sceneChanged) {
    sceneChanged = false;
    d->mMouseDown = true;
    foreach(Object * object, d->mDisplayList) {
      if (object != 0 && object->item() != 0 && object->item()->mItemType == Swiff::ItemButton) {
        Button *button = static_cast<Button *>(object->item());
        if (button->contains(object->matrix(), position)) {
          button->processTransition(Swiff::OverUpToOverDown);
          sceneChanged = true;
          // only the top-most button should be pressed
          return;
        }
      }
    }
  }

  void Sprite::mouseReleased(const QPointF &position, bool &sceneChanged) {
    sceneChanged = false;
    d->mMouseDown = false;
    foreach(Object * object, d->mDisplayList) {
      if (object != 0 && object->item() != 0 && object->item()->mItemType == Swiff::ItemButton) {
        Button *button = static_cast<Button *>(object->item());
        if (button->contains(object->matrix(), position)) {
          button->processTransition(Swiff::OverDownToOverUp);
          sceneChanged = true;
        } else {
          if (button->state() == Swiff::OverUp) {
            button->processTransition(Swiff::OutDownToIdle);
            sceneChanged = true;
          }
        }
      }
    }
  }

  void Sprite::processTags(const QVector<Tag *> &tags) {
    foreach(Tag * tag, tags) {
      switch (tag->id()) {
      case TagPlaceObject: {
          PlaceObject *placeObject = static_cast<PlaceObject *>(tag);
          if (d->mDocument->dictionary()[placeObject->mCharacterId] != 0) {
            Object *object = new Object();
            Item *item = d->mDocument->dictionary()[placeObject->mCharacterId];
            if (item->mItemType == Swiff::ItemButton) {
              Button *button = reinterpret_cast<Button *>(item);
              object->setItem(button->clone());
            } else
              object->setItem(d->mDocument->dictionary()[placeObject->mCharacterId]);
            // clean any previously existing object
            if (d->mDisplayList[placeObject->mDepth] != 0) {
              delete d->mDisplayList.take(placeObject->mDepth);
            }
            // put the object on the playlist
            d->mDisplayList[placeObject->mDepth] = object;
            object->setMatrix(placeObject->mMatrix);
            if (placeObject->hasColorTransform()) {
              object->setColorTransform(placeObject->mColorTransform);
            }
            if (object->item()->mItemType == Swiff::ItemSprite) {
              static_cast<Sprite *>(object->item())->play();
            }
          }
        }
        break;
      case TagPlaceObject2: {
          PlaceObject2 *placeObject = static_cast<PlaceObject2*>(tag);
          if (placeObject->hasClipDepth()) {
            // TODO: Implement clipping
          }
          Object *object = 0;
          if (placeObject->hasMove()) {
            object = d->mDisplayList[placeObject->mDepth];
          } else {
            object = new Object();
            d->mDisplayList[placeObject->mDepth] = object;
          }
          if (placeObject->hasCharacter()) {
            // dont put null items into the display list
            if (d->mDocument->dictionary()[placeObject->mCharacterId] != 0) {
              Item *item = d->mDocument->dictionary()[placeObject->mCharacterId];
              if (item->mItemType == Swiff::ItemButton) {
                Button *button = reinterpret_cast<Button *>(item);
                object->setItem(button->clone());
              } else
                object->setItem(d->mDocument->dictionary()[placeObject->mCharacterId]);
            }
          }
          if (object == 0 || object->item() == 0) {
            delete d->mDisplayList.take(placeObject->mDepth);
          } else {
            if (placeObject->hasMatrix()) {
              object->setMatrix(placeObject->mMatrix);
            }
            if (placeObject->hasColorTransform()) {
              object->setColorTransform(placeObject->mColorTransform);
            }
            if (object->item()->mItemType == Swiff::ItemSprite) {
              Sprite *sprite = static_cast<Sprite *>(object->item());
              if (placeObject->hasName()) {
                sprite->d->mName = placeObject->mObjectName;
              }
              sprite->play();
            }
          }
        }
        break;
      case TagPlaceObject3: {
          PlaceObject3 *placeObject = static_cast<PlaceObject3*>(tag);
          if (placeObject->hasClipDepth()) {
            // TODO: Implement clipping
          }
          Object *object = 0;
          if (placeObject->hasMove()) {
            object = d->mDisplayList[placeObject->mDepth];
          } else {
            object = new Object();
            d->mDisplayList[placeObject->mDepth] = object;
          }
          if (placeObject->hasCharacter()) {
            // dont put null items into the display list
            if (d->mDocument->dictionary()[placeObject->mCharacterId] != 0) {
              Item *item = d->mDocument->dictionary()[placeObject->mCharacterId];
              if (item->mItemType == Swiff::ItemButton) {
                Button *button = reinterpret_cast<Button *>(item);
                object->setItem(button->clone());
              } else
                object->setItem(d->mDocument->dictionary()[placeObject->mCharacterId]);
            }
          }
          if (object == 0 || object->item() == 0) {
            delete d->mDisplayList.take(placeObject->mDepth);
          } else {
            if (placeObject->hasMatrix()) {
              object->setMatrix(placeObject->mMatrix);
            }
            if (placeObject->hasColorTransform()) {
              object->setColorTransform(placeObject->mColorTransform);
            }
            if (object->item()->mItemType == Swiff::ItemSprite) {
              Sprite *sprite = static_cast<Sprite *>(object->item());
              if (placeObject->hasName()) {
                sprite->d->mName = placeObject->mObjectName;
              }
              sprite->play();
            }
          }
        }
        break;
      case TagRemoveObject: {
          RemoveObject *removeObject = static_cast<RemoveObject *>(tag);
          Object *object = d->mDisplayList.take(removeObject->mDepth);
          if (object != 0 && object->item() != 0 && object->item()->mItemType == Swiff::ItemSprite) {
            Sprite *sprite = static_cast<Sprite *>(object->item());
            sprite->stop();
            sprite->d->mName.clear();
          }
          delete object;
        }
        break;
      case TagRemoveObject2: {
          RemoveObject2 *removeObject = static_cast<RemoveObject2 *>(tag);
          Object *object = d->mDisplayList.take(removeObject->mDepth);
          if (object != 0 && object->item() != 0 && object->item()->mItemType == Swiff::ItemSprite) {
            Sprite *sprite = static_cast<Sprite *>(object->item());
            sprite->stop();
            sprite->d->mName.clear();
          }
          delete object;
        }
        break;
      case TagSetBackgroundColor: {
          d->mBackgroundBrush = QBrush(static_cast<SetBackgroundColor*>(tag)->color());
        }
        break;
      default:
        // do nothing
        break;
      }
    }
  }
}
