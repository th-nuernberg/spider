#ifndef __MAINWINDOW_HPP__7C3D1822_632F_4DE0_AF8F_F66E3E2AB551
#define __MAINWINDOW_HPP__7C3D1822_632F_4DE0_AF8F_F66E3E2AB551

#include <QMainWindow>

class QMdiArea;
class QMdiSubWindow;
class QAction;
class QMenu;
class QToolBar;
class QTextEdit;

namespace nst {

  namespace gui {

    class MainWindow : public QMainWindow
    {
      Q_OBJECT

    public:
      MainWindow(QWidget *parent = 0);
      virtual ~MainWindow();

    public slots:
        void addDVSTrackerControl();
        void addLogo();


    private:
      QMdiArea *_mdi = nullptr;
      QMdiSubWindow* _win = nullptr;
      QMenu *_mnuFile = nullptr;
      QAction *_actClose = nullptr;
    };


  }} // nst::gui::

#endif /* __MAINWINDOW_HPP__7C3D1822_632F_4DE0_AF8F_F66E3E2AB551 */

