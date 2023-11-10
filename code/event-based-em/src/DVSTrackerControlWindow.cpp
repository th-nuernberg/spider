#include "inc/DVSTrackerControlWindow.hpp"

#include <iostream>

#include <QCloseEvent>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QFrame>
#include <QPushButton>
#include <QCheckBox>
#include <QMdiArea>
#include <QComboBox>
#include <QDoubleValidator>
#include <QIntValidator>
#include <QSizePolicy>
#include <QDateTime>
#include <QFileDialog>
#include <QLabel>
#include <algorithm>

#include "inc/Utils.hpp"
#include "inc/DVSTrackerControl.hpp"
#include "inc/Commands.hpp"
#include "inc/UserFunction.hpp"
#include "inc/DVSVisualizerWindow.hpp"
#include "inc/DVSTrackerVisualizerWindow.hpp"
#include "inc/qcustomplot.h"

// user function vars
double rate_angle = 0.02f;
double rate_speed =1.0f;
int smoothing_is_closed_loop = 1;

// delay for replay
unsigned long replayDelay = 1;

namespace nst {

namespace gui {

DVSTrackerControlWindow::
DVSTrackerControlWindow(QWidget *parent, Qt::WindowFlags flags)
    : QMdiSubWindow(parent, flags)
{
    // create control and hook into signals
    _control = new DVSTrackerControl();
    connect(_control, &DVSTrackerControl::connected, this, &DVSTrackerControlWindow::onControlConnected);
    connect(_control, &DVSTrackerControl::disconnected, this, &DVSTrackerControlWindow::onControlDisconnected);

    // window frame
    this->setWindowTitle("DVS Tracker Control");
    this->setFixedSize(1000, 280);

    // create GUI
    _centralWidget = new QFrame(this);
    layout()->addWidget(_centralWidget);

    QGridLayout *layout = new QGridLayout();
    layout->addWidget(new QLabel("Port: ", _centralWidget));

    int row = 0;

    // connectivity
    _editPort = new QLineEdit("/dev/ttyUSB0", _centralWidget);
    layout->addWidget(_editPort, row, 1);

    _btnConnect = new QPushButton("Connect", _centralWidget);
    layout->addWidget(_btnConnect, row, 2);
    connect(_btnConnect, &QPushButton::clicked, this, &DVSTrackerControlWindow::onBtnConnectClicked);

    ++row;

    // recording functionality
    layout->addWidget(new QLabel("Output File: ", _centralWidget));
    QDateTime recFileTime = QDateTime::currentDateTime();
    _editFileOutName = "dvstracker_rec__"+recFileTime.toString("dd_MM_yyyy__hh_mm_ss")+".dvs";
    _editFileOut = new QLineEdit(_editFileOutName, _centralWidget);
    layout->addWidget(_editFileOut, row, 1);

    _btnRecord= new QPushButton("Record", _centralWidget);
    layout->addWidget(_btnRecord, row, 2);
    connect(_btnRecord, &QPushButton::clicked, this, &DVSTrackerControlWindow::onBtnRecordClicked);

    ++row;

    // replaying functionality using a new dialog
    layout->addWidget(new QLabel("Input File: ", _centralWidget));

    _editFileIn = new QLineEdit("...", _centralWidget);
    layout->addWidget(_editFileIn, row, 1);

    _btnReplay= new QPushButton("Replay", _centralWidget);
    layout->addWidget(_btnReplay, row, 2);
    connect(_btnReplay, &QPushButton::clicked, this, &DVSTrackerControlWindow::onBtnReplayClicked);

    ++row;

    // user function
    _cbUserFunction = new QCheckBox("", _centralWidget);
    _cbUserFunction->setCheckState(Qt::Unchecked);
    layout->addWidget(_cbUserFunction, row, 0, 1, 1);
    connect(_cbUserFunction, &QCheckBox::stateChanged, this, &DVSTrackerControlWindow::onCbUserFunctionStateChanged);

    _cmbUserFunction = new QComboBox(_centralWidget);
    for (size_t i = 0; i < LENGTH(user_functions); i++)
        _cmbUserFunction->addItem(user_functions[i].name);
    _cmbUserFunction->setEnabled(false);
    layout->addWidget(_cmbUserFunction, row, 1, 1, 2);
    void(QComboBox::*cmbsignal)(int) = &QComboBox::currentIndexChanged;
    connect(_cmbUserFunction, cmbsignal, this, &DVSTrackerControlWindow::onCmbUserFunctionIndexChanged);

    ++row;

    // algorithm parametrizaton

    // user function vars
    rate_angle = 0.02;
    rate_speed = 1.0;
    smoothing_is_closed_loop  = 0;

    // rate angle increment/decrement
    _paramRateAngle = new QLabel("Rate angle", _centralWidget);
    layout->addWidget(_paramRateAngle, row, 0);
    _btnParamRateAngleInc = new QPushButton("+", _centralWidget);
    _btnParamRateAngleInc->setFixedSize(40, 20);
    layout->addWidget(_btnParamRateAngleInc, row, 1, Qt::AlignLeft);
    _btnParamRateAngleDec = new QPushButton("-", _centralWidget);
    layout->addWidget(_btnParamRateAngleDec, row, 1, Qt::AlignCenter);
    _btnParamRateAngleDec->setFixedSize(40, 20);
    _valRateAngle = new QLabel ("Val: ", _centralWidget);
    layout->addWidget(_valRateAngle, row, 2, Qt::AlignRight);
    _valRateAngle->setNum(0.02f);
    connect(_btnParamRateAngleInc, &QPushButton::clicked, this, &DVSTrackerControlWindow::onParamRateAngleIncClicked);
    connect(_btnParamRateAngleDec, &QPushButton::clicked, this, &DVSTrackerControlWindow::onParamRateAngleDecClicked);

    ++row;

    // rate speed increment/decrement
    _paramRateSpeed = new QLabel("Rate speed", _centralWidget);
    layout->addWidget(_paramRateSpeed, row, 0);
    _btnParamRateSpeedInc = new QPushButton("+", _centralWidget);
    _btnParamRateSpeedInc->setFixedSize(40, 20);
    layout->addWidget(_btnParamRateSpeedInc, row, 1, Qt::AlignLeft);
    _btnParamRateSpeedDec = new QPushButton("-", _centralWidget);
    _btnParamRateSpeedDec->setFixedSize(40, 20);
    layout->addWidget(_btnParamRateSpeedDec, row, 1, Qt::AlignCenter);
    _valRateSpeed= new QLabel("Val: ", _centralWidget);
    layout->addWidget(_valRateSpeed, row, 2, Qt::AlignRight);
    _valRateSpeed->setNum(1.0f);
    connect(_btnParamRateSpeedInc, &QPushButton::clicked, this, &DVSTrackerControlWindow::onParamRateSpeedIncClicked);
    connect(_btnParamRateSpeedDec, &QPushButton::clicked, this, &DVSTrackerControlWindow::onParamRateSpeedDecClicked);

    ++row;

    // replay speed increment/decrement
    _paramReplaySpeed = new QLabel("Replay speed", _centralWidget);
    layout->addWidget(_paramReplaySpeed, row, 0);

    _btnParamReplaySpeedInc = new QPushButton("-", _centralWidget);
    _btnParamReplaySpeedInc->setFixedSize(200, 20);
    layout->addWidget(_btnParamReplaySpeedInc, row, 1, Qt::AlignLeading);

    _btnParamReplaySpeedStop = new QPushButton("#", _centralWidget);
    _btnParamReplaySpeedStop->setFixedSize(200, 20);
    layout->addWidget(_btnParamReplaySpeedStop, row, 1, Qt::AlignCenter);

    _btnParamReplaySpeedPlay = new QPushButton(">", _centralWidget);
    _btnParamReplaySpeedPlay->setFixedSize(200, 20);
    layout->addWidget(_btnParamReplaySpeedPlay, row, 1, Qt::AlignTrailing);

    _btnParamReplaySpeedDec = new QPushButton("+", _centralWidget);
    _btnParamReplaySpeedDec->setFixedSize(200, 20);
    layout->addWidget(_btnParamReplaySpeedDec, row, 2, Qt::AlignRight);

    connect(_btnParamReplaySpeedInc, &QPushButton::clicked, this, &DVSTrackerControlWindow::onParamReplaySpeedIncClicked);
    connect(_btnParamReplaySpeedStop, &QPushButton::clicked, this, &DVSTrackerControlWindow::onParamReplaySpeedStopClicked);
    connect(_btnParamReplaySpeedPlay, &QPushButton::clicked, this, &DVSTrackerControlWindow::onParamReplaySpeedPlayClicked);
    connect(_btnParamReplaySpeedDec, &QPushButton::clicked, this, &DVSTrackerControlWindow::onParamReplaySpeedDecClicked);

    ++row;

    // smoothing in closed loop enable / disable
    _paramSmoothingClosedLoop = new QLabel("Closed-loop smoothing", _centralWidget);
    layout->addWidget(_paramSmoothingClosedLoop, row, 0);
    _cbParamSmoothingClosedLoop = new QCheckBox("", _centralWidget);
    _cbParamSmoothingClosedLoop->setCheckState(Qt::Unchecked);
    layout->addWidget(_cbParamSmoothingClosedLoop, row, 1, 1, 1);
    connect(_cbParamSmoothingClosedLoop, &QCheckBox::stateChanged, this, &DVSTrackerControlWindow::onParamSmoothingClosedLoop);
    _lbSmoothing= new QLabel("Disabled", _centralWidget);
    layout->addWidget(_lbSmoothing, row, 1, Qt::AlignRight);

    ++row;

    // time stamp plotting and analysis
    _paramTimestamp = new QLabel("Time stamp analysis", _centralWidget);
    layout->addWidget(_paramTimestamp, row, 0);
    _btnParamTimestamp = new QPushButton("Timestamp plot", _centralWidget);
    _btnParamTimestamp->setFixedSize(150, 20);
    layout->addWidget(_btnParamTimestamp, row, 1, Qt::AlignLeading);
    connect(_btnParamTimestamp, &QPushButton::clicked, this, &DVSTrackerControlWindow::onParamTimestampClicked);

    ++row;

    // event visualization
    _winEventVisualizer = new DVSVisualizerWindow(_control, parent);
    _winEventVisualizer->resize(500, 500);
    _winEventVisualizer->show();
    _winEventVisualizer->move(0, 280);
    _winEventVisualizer->setWindowTitle("DVS Events visualizer");

    // tracker visualization
    _winTrackerVisualizer = new DVSTrackerVisualizerWindow(_control, parent);
    _winTrackerVisualizer->resize(500, 500);
    _winTrackerVisualizer->show();
    _winTrackerVisualizer->move(500, 280);
    _winTrackerVisualizer->setWindowTitle("Tracker visualizer");

    // create the timestamp visualization window
    _evTmsPlot = new QCustomPlot;
    // connect slots that takes care that when an axis is selected, only that direction can be dragged and zoomed:
    connect(_evTmsPlot, SIGNAL(mouseWheel(QWheelEvent*)), this, SLOT(onParamTimestampMouseWheel()));

    _centralWidget->setLayout(layout);
    setAttribute(Qt::WA_DeleteOnClose);
}


DVSTrackerControlWindow::
~DVSTrackerControlWindow()
{
}

void DVSTrackerControlWindow::
closeEvent(QCloseEvent *ev)
{
    emit closing(this);
    QMdiSubWindow::closeEvent(ev);
}


void DVSTrackerControlWindow::
onBtnConnectClicked()
{
    if (!_control->isConnected()){
        _control->connectDVSTracker(_editPort->text());
    }
    else{
        _control->disconnectDVSTracker();
    }
}

void DVSTrackerControlWindow::
onBtnRecordClicked()
{
    if (!_control->isConnected()){
        _control->connectDVSTracker(_editPort->text());
        _control->connectDVSTracker(_editFileOut->text());
    }
    else{
        _control->disconnectDVSTracker();
    }
}

void DVSTrackerControlWindow::
onBtnReplayClicked()
{
    // create the new file dialog
    QString fileInName = QFileDialog::getOpenFileName(this, tr("Open DVS recording"), "./", tr("DVS Files (*.dvs)"));
    _editFileIn->setText(fileInName);
    if (!_control->isConnected()){
        _control->connectDVSTracker(_editFileIn->text());
    }
    else{
        _control->disconnectDVSTracker();
    }
    // set the default replay delay
    replayDelay = 1; // ms
}

void DVSTrackerControlWindow::
onParamRateAngleIncClicked()
{
    rate_angle *= 1.01;
    _valRateAngle->setNum(rate_angle);
}

void DVSTrackerControlWindow::
onParamRateAngleDecClicked()
{
    rate_angle *= 0.99;
    _valRateAngle->setNum(rate_angle);
}

void DVSTrackerControlWindow::
onParamRateSpeedIncClicked()
{
    rate_speed *= 1.01;
    _valRateSpeed->setNum(rate_speed);
}

void DVSTrackerControlWindow::
onParamRateSpeedDecClicked()
{
    rate_speed *= 0.99;
    _valRateSpeed->setNum(rate_speed);
}

void DVSTrackerControlWindow::
onParamReplaySpeedIncClicked()
{
    replayDelay-=DEF_REPLAY_DELAY;
    replayDelay%=ULONG_MAX;
}

void DVSTrackerControlWindow::
onParamReplaySpeedDecClicked()
{
    replayDelay+=DEF_REPLAY_DELAY;
}

void DVSTrackerControlWindow::
onParamReplaySpeedPlayClicked()
{
    replayDelay=DEF_REPLAY_DELAY;
}


void DVSTrackerControlWindow::
onParamReplaySpeedStopClicked()
{
    replayDelay=0;
}

void DVSTrackerControlWindow::
onParamTimestampClicked()
{
    // prepare display window
    _evTmsPlot->show();
    _evTmsPlot->setGeometry(400, 250, 542, 390);
    _evTmsPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectAxes |
                                      QCP::iSelectLegend | QCP::iSelectPlottables);
    _evTmsPlot->axisRect()->setupFullAxesBox();
    // get timetamp data from event buffer
    auto evBuf = this->_control->getEvBuffer();
    QVector<double> x(evBuf.size());
    QVector<double> y(evBuf.size());
    for (uint idx = 0; idx<evBuf.size(); ++idx)
    {
        x[idx] = idx;                // event idx
        y[idx] = evBuf.at(idx)->t;   // timestamp
    }
    // create graph and assign data to it
    _evTmsPlot->addGraph();
    _evTmsPlot->graph(0)->setData(x, y);
    // give the axes some labels:
    _evTmsPlot->xAxis->setLabel("timestamp");
    _evTmsPlot->yAxis->setLabel("event#");
    // set axes ranges, so we see all data
    _evTmsPlot->xAxis->setRange(0, evBuf.size());
    _evTmsPlot->yAxis->setRange(0, *std::max_element(y.constBegin(), y.constEnd()));
    _evTmsPlot->replot();
}

void DVSTrackerControlWindow::
onParamTimestampMouseWheel()
{
    // if an axis is selected, only allow the direction of that axis to be zoomed
    // if no axis is selected, both directions may be zoomed

    if (this->_evTmsPlot->xAxis->selectedParts().testFlag(QCPAxis::spAxis))
      this->_evTmsPlot->axisRect()->setRangeZoom(this->_evTmsPlot->xAxis->orientation());
    else if (this->_evTmsPlot->yAxis->selectedParts().testFlag(QCPAxis::spAxis))
      this->_evTmsPlot->axisRect()->setRangeZoom(this->_evTmsPlot->yAxis->orientation());
    else
      this->_evTmsPlot->axisRect()->setRangeZoom(Qt::Horizontal|Qt::Vertical);
}

void DVSTrackerControlWindow::
onControlConnected()
{
    _btnConnect->setText("Disconnect");
    _editPort->setReadOnly(true);
    _editPort->setEnabled(false);

    // set a user function
    if (_cbUserFunction->checkState() == Qt::Checked) {
        int index = _cmbUserFunction->currentIndex();
        if (index >= 0){
            setUserFunction(static_cast<unsigned>(index));
        }
        else{
            unsetUserFunction();
        }
    }
    else{
        unsetUserFunction();
    }
}


void DVSTrackerControlWindow::
onControlDisconnected()
{
    _editPort->setReadOnly(false);
    _editPort->setEnabled(true);
    _btnConnect->setText("Connect");
}

void DVSTrackerControlWindow::
onCbUserFunctionStateChanged(int state)
{
    if (state == Qt::Checked) {
        _cmbUserFunction->setEnabled(true);
        setUserFunction(_cmbUserFunction->currentIndex());
    }
    else {
        _cmbUserFunction->setEnabled(false);
        unsetUserFunction();
    }
}

void DVSTrackerControlWindow::
onParamSmoothingClosedLoop(int state)
{
    if (state == Qt::Checked) {
        // set smoothing in the closed loop
        smoothing_is_closed_loop = 1;
        _lbSmoothing->setText("Enabled");
    }
    else {
        // unset smoothing in closed loop
        smoothing_is_closed_loop = 0;
        _lbSmoothing->setText("Disabled");
    }
}

void DVSTrackerControlWindow::
onCmbUserFunctionIndexChanged(int index)
{
    // check the value. if no item has been selected we want to unset the user function
    if (index < 0)
        unsetUserFunction();
    else
        setUserFunction(static_cast<unsigned>(index));
}

void DVSTrackerControlWindow::
setUserFunction(unsigned index)
{
    if (index < LENGTH(user_functions))
        _control->setUserFunction(&user_functions[index]);
}

void DVSTrackerControlWindow::
unsetUserFunction()
{
    _control->unsetUserFunction();
}


}} // nst::gui
