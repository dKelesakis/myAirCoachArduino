#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <qcustomplot.h>
#include "qextserialport.h"
#include "qextserialport.h"
#include "qextserialenumerator.h"
#include <QDebug>
#include <QList>
#include <QDebug>
#include <QDesktopWidget>
#include <QScreen>
#include <QMessageBox>
#include <QMetaEnum>
#include <QDialog>
#include "qledindicator.h"
#include <complex>

#include <QMainWindow>
#include <QWidget>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QTemporaryFile>

#include "AppStateData.h"

QT_BEGIN_NAMESPACE
class QLabel;
class QSlider;
class QListWidget;
class QComboBox;
class QCheckBox;
class QLineEdit;
class QPushButton;
class QGraphicsView;
class QGraphicsScene;
class QListWidgetItem;
class QFrame;
QT_END_NAMESPACE

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();


    void keyPressEvent( QKeyEvent *event );

    typedef std::complex<double> Complex;

    void setupRealtimeRawDataPlot(QCustomPlot *customPlot);
    void setupFFTDataPlot(QCustomPlot *customPlot,double data);
    void setupStyledDemo(QCustomPlot *customPlot);
    void setupAdvancedAxesDemo(QCustomPlot *customPlot);
    void setupColorMapDemo(QCustomPlot *customPlot);
    void ParseSignalFile();

    void setupPlayground(QCustomPlot *customPlot);
    void init_port(); // port initialisation function
    void ReadFromBoard();
    void SendCommandToBoard(QString Data);
    void listSerialPorts();
    int FFTmain();
    void shiftData(double *data, int len, double newValue);
    void logData();

    //fft function declration
    void sdft_init_coeffs();
    void sdft_init();
    void sdft_add_data();
    void sdft();
    void isdft();
    void ft();
    double mag(Complex& c);
    void powr_spectrum(double *powr);


    void onReceiveMsg();
protected:
    void changeEvent(QEvent *e);

private slots:
    void realtimeDataSlot();
//  void screenShot();
//  void allScreenShots();
//  void transmitCmd(int value); // sending function
    void on_realtimeBtn_clicked();
    //void onReadyReadFromBoard();

    //tab 3 page
    void on_mainAddButton_clicked();
    void on_mainRemoveButton_clicked();
    void on_mainCatListWidget_doubleClicked();
    void on_mainCatListWidget_itemChanged(QListWidgetItem *item);
    void on_mainCatListWidget_currentItemChanged( QListWidgetItem *current );

    void on_subAddButton_clicked();
    void on_subRemoveButton_clicked();
    void on_subCatListWidget_doubleClicked();
    void on_subCatListWidget_itemChanged(QListWidgetItem *item);
    void on_subCatListWidget_currentItemChanged(QListWidgetItem *current);

    void on_saveButton_clicked();
    void on_saveAsButton_clicked();
    void on_loadButton_clicked();

    void on_addFilesButton_clicked();
    void on_fileListWidget_itemSelectionChanged();

    void on_colorComboBox_currentIndexChanged();
    void on_yearComboBox_currentIndexChanged();
    void on_sizeSlider_valueChanged( int value );
    void on_brokenCheckBox_toggled(bool checked);
    void on_missingCheckBox_toggled(bool checked);
    void on_batteriesCheckBox_toggled(bool checked);



private Q_SLOTS :
    void on_connectOrDisconnectBtn_clicked();
    void onPortNameChanged(const QString &name);
    void onBaudRateChanged(int idx);
    void onParityChanged(int idx);
    void onDataBitsChanged(int idx);
    void onStopBitsChanged(int idx);
    void onQueryModeChanged(int idx);
    void onTimeoutChanged(int val);
    void onReadyRead();
    void onPortAddedOrRemoved();
    void on_stopBtn_clicked();
    void on_commandLinkBtnClearConsole_clicked();
    void on_toolButton_browse_clicked();
    void radioBtnbox_randData();
    void radioBtnbox_offline();

    void on_SendToSerialPort_clicked();



    void on_comboBox_currentIndexChanged(int index);


    void on_comboBox_3_currentIndexChanged(int index);

    void on_comboBox_4_currentIndexChanged(int index);

    void on_comboBox_5_currentIndexChanged(int index);


private:

    Ui::MainWindow *ui;
    QString demoName;
    QTimer dataTimer;
    QTimer dataTime_fft;
    QCPItemTracer *itemDemoPhaseTracer;
    int currentDemoIndex;
    QextSerialPort *port;
    QTimer *timer;
    QextSerialEnumerator *enumerator;
    double globalSerialDataValue =0;
    int prevTempAdc;
    int currentTempAdc;
    QLedIndicator *LED0;
    QLedIndicator *LED1;
    QLedIndicator *LED2;
    QFile logFile;
    QTextStream logSerialDataStream;
    QList <double> serialDataList;

    //FFT Data Source
    static const int N = 512;
    // input signal
    int in[N];

    // frequencies of input signal after ft
    // Size increased by one because the optimized sdft code writes data to freqs[N]
    Complex freqs[N+1];

    // output signal after inverse ft of freqs
    double out1[N];
    double out2[N];

    // forward coeffs -2 PI e^iw -- normalized (divided by N)
    Complex coeffs[N];
    // inverse coeffs 2 PI e^iw
    Complex icoeffs[N];

    // global index for input and output signals
    int idx;

    int oldest_data, newest_data;
    int globalSerialDataCounter;

    //tab page 3
    void            loadAndParseJsonFile( const QString path );
    void            serializeAndSaveJsonFile( const QString path );
    void            saveSettings();
    ImageData*      getCurrentImageData();

    //Ui::MainWindow  *ui;
    QLabel          *ui_imagePreview;
    QLabel          *ui_pathLabel;
    QGraphicsScene  *ui_graphicsScene;
    QLabel          *ui_sizeLabel;
    QListWidget     *ui_mainCatListWidget;
    QListWidget     *ui_subCatListWidget;
    QFrame          *ui_subCatButtonLayout;
    QListWidget     *ui_fileListWidget;
    QPushButton     *ui_saveAsButton;
    QPushButton     *ui_saveButton;
    QSlider         *ui_sizeSlider;
    QComboBox       *ui_colorComboBox;
    QComboBox       *ui_yearComboBox;
    QCheckBox       *ui_brokenCheckBox;
    QCheckBox       *ui_missingCheckBox;
    QCheckBox       *ui_batteriesCheckBox;
    QLabel          *label_sensor2;
    // Temp file things
    QTemporaryFile  m_tempFile;
    QString         m_loadedMetaFileName;
    QJsonObject     m_tempJson;
    QJsonArray      m_fileListJsonArray;
    QString         m_lastOpenMetafile;

    AppStateData    *m_appStateData;
};

#endif // MAINWINDOW_H








