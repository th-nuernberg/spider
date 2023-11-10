#include "inc/DVSTrackerWidget.hpp"
#include "inc/Datatypes.hpp"
#include "inc/Utils.hpp"

#include <QRect>
#include <QPainter>

namespace  nst {

  struct TrackerEvent;

  namespace gui{

    DVSTrackerWidget::DVSTrackerWidget(QWidget *parent) : QWidget(parent)
    {
      _trackerImage = new QImage(128, 128, QImage::Format_RGB32);
      _decayFactor = 0.4;

      _timer.setTimerType(Qt::PreciseTimer);
      connect(&_timer, &QTimer::timeout, this, &DVSTrackerWidget::decayImage);
      _timer.setInterval(VISUAL_UPDATE);
      _timer.start();

      setAttribute(Qt::WA_OpaquePaintEvent, true);
      setAttribute(Qt::WA_NoSystemBackground, true);
    }

    DVSTrackerWidget::~DVSTrackerWidget()
    {
      delete _trackerImage;
    }

    void DVSTrackerWidget::
    decayPixel(unsigned *p)
    {
      *p = qRgb(qRed(*p) * _decayFactor,
                qBlue(*p) * _decayFactor,
                qGreen(*p) * _decayFactor);
    }

    void DVSTrackerWidget::
    decayImage()
    {
      unsigned *bits = (unsigned*) _trackerImage->bits();
      unsigned N = _trackerImage->width()*_trackerImage->height();
      for(unsigned i = 0; i<N; i++)
        decayPixel(bits++);
      update();
    }

    void DVSTrackerWidget::
    paintEvent(QPaintEvent *)
    {
      QPainter painter(this);
      QRect rect(0, 0, geometry().width(), geometry().height());
      painter.drawImage(rect, *_trackerImage);
    }

    void DVSTrackerWidget::
    newPosition(TrackerEvent *trkev)
    {
      QPainter p(this->_trackerImage);
      int lineSize = 0;

      if(trkev)
        {
          // plot the blue segment
          p.setPen(QPen(Qt::blue, lineSize));
        p.drawLine(trkev->bluesegments[0], trkev->bluesegments[1], trkev->bluesegments[2], trkev->bluesegments[3]);
          // plot the red segment
          p.setPen(QPen(Qt::red, lineSize));
        p.drawLine(trkev->redsegments[0], trkev->redsegments[1], trkev->redsegments[2], trkev->redsegments[3]);
          // plot the yellow segment
          p.setPen(QPen(Qt::yellow, lineSize));
        p.drawLine(trkev->yellowsegments[0], trkev->yellowsegments[1], trkev->yellowsegments[2], trkev->yellowsegments[3]);
          // plot the center with the right color
          if (trkev->centers[2] == 1.0)
          {
              p.setPen(QPen(Qt::blue, lineSize));
              p.drawPoint(trkev->centers[0], trkev->centers[1]);
          }
          else if (trkev->centers[2] == 2.0)
          {
              p.setPen(QPen(Qt::red, lineSize));
              p.drawPoint(trkev->centers[0], trkev->centers[1]);
          }
          else if (trkev->centers[2] == 3.0)
          {
              p.setPen(QPen(Qt::yellow, lineSize));
              p.drawPoint(trkev->centers[0], trkev->centers[1]);
          }
          else
          {
              p.setPen(QPen(Qt::darkGray, lineSize));
              p.drawPoint(trkev->centers[0], trkev->centers[1]);
          }

        }
        update();
    }
  }
} // nst::gui::

