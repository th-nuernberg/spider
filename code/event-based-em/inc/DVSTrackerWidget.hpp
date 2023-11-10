#ifndef DVSTRACKERWIDGET_H
#define DVSTRACKERWIDGET_H

#include <memory>
#include <QWidget>
#include <QImage>
#include <QPaintEvent>
#include <QTimer>

namespace nst{

  // forward decls
  struct TrackerEvent;

  namespace gui{

    class DVSTrackerWidget : public QWidget
    {
      Q_OBJECT
    public:
      explicit DVSTrackerWidget(QWidget *parent = nullptr);
      ~DVSTrackerWidget();

    public slots:
      void paintEvent(QPaintEvent *event);
      void newPosition(TrackerEvent *trkev);
      void decayImage();

    private:
      void decayPixel(unsigned *p);

      QImage *_trackerImage = nullptr;
      float _decayFactor;
      QTimer _timer;
    };
  }
}
#endif // DVSTRACKERWIDGET_H
