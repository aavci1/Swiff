#ifndef SWIFFSPRITE_H
#define SWIFFSPRITE_H

#include "SwiffItem.h"

#include <QVector>

namespace Swiff {
  class BitStream;
  class Document;
  class Frame;
  class Sound;
  class SpritePrivate;
  class Tag;

  enum SpriteState {
    SpritePlaying = 1,
    SpritePaused = 3,
    SpriteStopped = 5
  };

  class Sprite : public QObject, public Item {
    Q_OBJECT
  public:
    Sprite(Document *document);
    ~Sprite();
  public:
    /** \copydoc Item::parse */
    void parse(BitStream &bitstream, int version);
    /** \copydoc Item::paint */
    void paint(QPainter &painter);
    /** \copydoc Item::bounds */
    QRect bounds() const;
    /** \copydoc Item::properties. */
    QVector<QPair<QString, QString> > properties() const;
    /** \returns frames. */
    QVector<Frame *> frames() const;
    /** \returns playback streams. */
    QVector<Sound *> streams() const;
    /** \returns Document. */
    Document *document() const;
    /** \returns Background brush. */
    QBrush backgroundBrush() const;
    /** \returns Current state of the sprite. */
    SpriteState state() const;
  public slots:
    /** Sets state of the sprite. */
    void setState(SpriteState state);
    /** Starts playing. */
    void play();
    /** Pauses playing. */
    void pause();
    /** Stops playing. */
    void stop();
    /** Rewinds the movie. */
    void rewind();
    /** Should be called when mouse moved. */
    void mouseMoved(const QPointF &position, bool &overButton, bool &sceneChanged);
    /** Should be called when mouse pressed. */
    void mousePressed(const QPointF &position, bool &sceneChanged);
    /** Should be called when mouse released. */
    void mouseReleased(const QPointF &position, bool &sceneChanged);
    /** Goes back to the previous frame. Uses gotoFrame. */
    void prevFrame();
    /** Proceeds to the next frame. Uses gotoFrame. */
    void nextFrame();
    /** Goest to the \p frame. */
    void gotoFramePtr(Frame *frame);
    /** Goes to the frame with number \param frameNo. */
    void gotoFrame(const u16 frameNo);
    /** Goes to the frame with \param frameLabel. */
    void gotoFrame(const QString &frameLabel);
  private:
    /** Processes tags given in the \param tags. */
    void processTags(const QVector<Tag *> &tags);
  signals:
    void update();
  private:
    SpritePrivate *d;
  };
}

#endif
