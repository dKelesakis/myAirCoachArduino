#include "mainwindow.h"
#include "AppStateData.h"
#include "ui_mainwindow.h"
#include "qextserialport.h"
#include "qextserialenumerator.h"
#include <stdio.h>
#include <unistd.h>
#include "print.h"
#include <QButtonGroup>
#include <QTimer>
#include <QtCore>
#include <QDebug>
#include <QDate>

\

#include <QDebug>
#include <QInputDialog>
#include <QListWidgetItem>
#include <QFileSystemModel>
#include <QJsonDocument>
#include <QGraphicsView>
#include <QJsonObject>
#include <QJsonArray>
#include <QFileDialog>
#include <QList>
#include <QtGui>
#include <QDir>
#include <QSettings>
#include <QShortcut>

//MainWindow::~MainWindow() { delete ui; }
//in order not to have redefinition

//--------------------------------------------------------------------------------------
MainWindow::MainWindow( QWidget *parent ) : QMainWindow( parent ), ui( new Ui::MainWindow )
{
     setWindowTitle("-_-");
    ui->setupUi( this );

    globalSerialDataCounter=0;
    sdft_init();
    foreach (QextPortInfo info, QextSerialEnumerator::getPorts())
        ui->portComboBox->addItem(info.portName);
    ui->portComboBox->setCurrentIndex(4);

    //make sure user can input their own port name!
    ui->portComboBox->setEditable(true);

    ui->baudRateComboBox->addItem("1200", BAUD1200);
    ui->baudRateComboBox->addItem("2400", BAUD2400);
    ui->baudRateComboBox->addItem("4800", BAUD4800);
    ui->baudRateComboBox->addItem("9600", BAUD9600);
    ui->baudRateComboBox->addItem("19200",BAUD19200);
    ui->baudRateComboBox->addItem("57600",BAUD57600);
    ui->baudRateComboBox->addItem("115200",BAUD115200);
    //ui->baudRateComboBox->addItem("1500000",BAUD1500000);
    ui->baudRateComboBox->setCurrentIndex(6);

    ui->parityComboBox->addItem("NONE", PAR_NONE);
    ui->parityComboBox->addItem("ODD", PAR_ODD);
    ui->parityComboBox->addItem("EVEN", PAR_EVEN);

    ui->flowComboBox->addItem("OFF");
    ui->flowComboBox->addItem("Xon" );
    ui->flowComboBox->addItem("Xoff");
    ui->flowComboBox->setCurrentIndex(0);

    ui->dataBitComboBox->addItem("5", DATA_5);
    ui->dataBitComboBox->addItem("6", DATA_6);
    ui->dataBitComboBox->addItem("7", DATA_7);
    ui->dataBitComboBox->addItem("8", DATA_8);
    ui->dataBitComboBox->setCurrentIndex(3);

    ui->StopbitComboBox->addItem("1", STOP_1);
    ui->StopbitComboBox->addItem("2", STOP_2);

    ui->queryModeComboBox->addItem("Polling", QextSerialPort::Polling);
    ui->queryModeComboBox->addItem("EventDriven", QextSerialPort::EventDriven);

    //ui->sendcommandBox->setEnabled(false);
    ui->tabWidget->setCurrentIndex(2);

    timer = new QTimer(this);
    timer->setInterval(5);

    enumerator = new QextSerialEnumerator(this);
    enumerator->setUpNotifications();


    PortSettings settings = {BAUD9600, DATA_8, PAR_NONE, STOP_1, FLOW_OFF, 10};
    port = new QextSerialPort(ui->portComboBox->currentText(), settings, QextSerialPort::Polling);


    ui->groupBox_command->setEnabled(false);

//    ui->board1Btn->setChecked(true);

    connect(ui->baudRateComboBox, SIGNAL(currentIndexChanged(int)), SLOT(onBaudRateChanged(int)));
    connect(ui->parityComboBox, SIGNAL(currentIndexChanged(int)), SLOT(onParityChanged(int)));
    connect(ui->dataBitComboBox, SIGNAL(currentIndexChanged(int)), SLOT(onDataBitsChanged(int)));
    connect(ui->StopbitComboBox, SIGNAL(currentIndexChanged(int)), SLOT(onStopBitsChanged(int)));
    connect(ui->queryModeComboBox, SIGNAL(currentIndexChanged(int)), SLOT(onQueryModeChanged(int)));
    connect(ui->timeoutComboBox, SIGNAL(valueChanged(int)), SLOT(onTimeoutChanged(int)));
    connect(ui->portComboBox, SIGNAL(editTextChanged(QString)), SLOT(onPortNameChanged(QString)));

    connect(timer, SIGNAL(timeout()), SLOT(onReadyRead()));
    connect(port, SIGNAL(readyRead()), SLOT(onReadyRead()));

    connect(enumerator, SIGNAL(deviceDiscovered(QextPortInfo)), SLOT(onPortAddedOrRemoved()));
    connect(enumerator, SIGNAL(deviceRemoved(QextPortInfo)), SLOT(onPortAddedOrRemoved()));

//    connect(ui->logCheckBox, SIGNAL(clicked()),SLOT(fileLogger()));
    connect(ui->radioButton_randData,SIGNAL(toggled(bool)),this,SLOT(radioBtnbox_randData()));
    connect(ui->radioButton_offline,SIGNAL(toggled(bool)),this,SLOT(radioBtnbox_offline()));
    connect(ui->radioButton_online,SIGNAL(toggled(bool)) ,this,SLOT(radioBtnbox_offline()));



    //tab 3 page
    // Get children we'll need
    ui_imagePreview         = findChild<QLabel*>( "pixmapLabel" );
    ui_pathLabel            = findChild<QLabel*>( "pathLabel" );
    ui_sizeLabel            = findChild<QLabel*>( "sizeSelectedLabel" );
    ui_mainCatListWidget    = findChild<QListWidget*>( "mainCatListWidget" );
    ui_subCatListWidget     = findChild<QListWidget*>( "subCatListWidget" );
    ui_subCatButtonLayout   = findChild<QFrame*>( "subCatButtonLayout_2" );
    ui_fileListWidget       = findChild<QListWidget*>( "fileListWidget" );
    ui_sizeSlider           = findChild<QSlider*>( "sizeSlider" );
    ui_colorComboBox        = findChild<QComboBox*>( "colorComboBox" );
    ui_yearComboBox         = findChild<QComboBox*>( "yearComboBox" );
    ui_brokenCheckBox       = findChild<QCheckBox*>( "brokenCheckBox" );
    ui_batteriesCheckBox    = findChild<QCheckBox*>( "batteriesCheckBox" );
    ui_missingCheckBox      = findChild<QCheckBox*>( "missingCheckBox" );
    ui_saveButton           = findChild<QPushButton*>( "saveButton" );
    ui_saveAsButton         = findChild<QPushButton*>( "saveAsButton" );

    ui_saveButton->setEnabled( false );
    ui_pathLabel->setText("");

    // Shortcut - Save
    QShortcut *saveShortcut = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_S), this);
    QObject::connect( saveShortcut, SIGNAL( activated() ), this, SLOT(on_saveButton_clicked()) );

    // Shortcut - Save As
    QShortcut *saveAsShortcut = new QShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_S), this);
    QObject::connect( saveAsShortcut, SIGNAL( activated() ), this, SLOT(on_saveAsButton_clicked()) );

    // Shortcut - Load
    QShortcut *openShortcut = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_O), this);
    QObject::connect( openShortcut, SIGNAL( activated() ), this, SLOT(on_loadButton_clicked()) );

    // Add years to combo box
    const int startingYear = 1960;
    for ( int i = startingYear; i <= QDate::currentDate().year(); i++ )
        ui_yearComboBox->addItem( QString::number( i ) );

    m_appStateData = new AppStateData();

    //COMMENTED DUE TO ERROR PRODUCED
    // Check for settings
    QSettings settings1( "Jeremy Abel", "Spectra" );
    if ( settings1.status() == QSettings::NoError )
    {
        m_lastOpenMetafile = settings1.value( "lastMetaFile", "" ).toString();

        // Load from last metafile
        //if ( m_lastOpenMetafile.length() > 0 )
          //  loadAndParseJsonFile( m_lastOpenMetafile );
    //}
}


}



void MainWindow::radioBtnbox_randData()
{
    if (ui->radioButton_randData->isChecked())
    {
        ui->groupBox_hardware->setEnabled(false);
        ui->groupBox_command->setEnabled(false);
    }

}

void MainWindow::radioBtnbox_offline()
{
    if (ui->radioButton_offline->isChecked())
    {
        ui->groupBox_offline->setEnabled(true);
//        ui->groupBox_online->setEnabled(false);
        ui->groupBox_hardware->setEnabled(false);
        ui->groupBox_command->setEnabled(false);
    }
    else {
        ui->groupBox_offline->setEnabled(false);
//        ui->groupBox_online->setEnabled(true);
        ui->groupBox_hardware->setEnabled(true);
        ui->groupBox_command->setEnabled(true);
    }
}

void MainWindow::on_realtimeBtn_clicked()
{
    setupRealtimeRawDataPlot(ui->customPlot);
    ui->customPlot->replot();

}

void MainWindow::setupPlayground(QCustomPlot *customPlot)
{
  Q_UNUSED(customPlot)
}

MainWindow::~MainWindow()
{
   //port->close(); //we close the port at the end of the program

   //port =NULL;
   if (ui->logCheckBox->isChecked())
      logData();
   delete ui;
   delete port;
}

void MainWindow::changeEvent(QEvent *e)
{
    QMainWindow::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}


//******************Signal Plot Segment*******************************

void MainWindow::setupRealtimeRawDataPlot(QCustomPlot *customPlot)
{
#if QT_VERSION < QT_VERSION_CHECK(4, 7, 0)
  QMessageBox::critical(this, "", "You're using Qt < 4.7, the realtime data demo needs functions that are available with Qt 4.7 to work properly");
#endif

  // include this section to fully disable antialiasing for higher performance:

  customPlot->setNotAntialiasedElements(QCP::aeAll);
  QFont font;
  font.setStyleStrategy(QFont::NoAntialias);
  customPlot->xAxis->setTickLabelFont(font);
  customPlot->yAxis->setTickLabelFont(font);
  customPlot->legend->setFont(font);

  customPlot->addGraph(); // blue line
  customPlot->graph(0)->setPen(QPen(Qt::blue));
  customPlot->graph(0)->setBrush(QBrush(QColor(240, 255, 200)));
  customPlot->graph(0)->setAntialiasedFill(false);
  customPlot->addGraph(); // red line
  customPlot->graph(1)->setPen(QPen(Qt::red));
  customPlot->graph(0)->setChannelFillGraph(customPlot->graph(1));

  customPlot->addGraph(); // blue dot
  customPlot->graph(2)->setPen(QPen(Qt::blue));
  customPlot->graph(2)->setLineStyle(QCPGraph::lsNone);
  customPlot->graph(2)->setScatterStyle(QCPScatterStyle::ssDisc);
  customPlot->addGraph(); // red dot
  customPlot->graph(3)->setPen(QPen(Qt::red));
  customPlot->graph(3)->setLineStyle(QCPGraph::lsNone);
  customPlot->graph(3)->setScatterStyle(QCPScatterStyle::ssDisc);

  customPlot->xAxis->setTickLabelType(QCPAxis::ltDateTime);
  customPlot->xAxis->setDateTimeFormat("hh:mm:ss");
  customPlot->xAxis->setAutoTickStep(false);
  customPlot->xAxis->setTickStep(2);
  customPlot->axisRect()->setupFullAxesBox();

  // make left and bottom axes transfer their ranges to right and top axes:
  connect(customPlot->xAxis, SIGNAL(rangeChanged(QCPRange)), customPlot->xAxis2, SLOT(setRange(QCPRange)));
  connect(customPlot->yAxis, SIGNAL(rangeChanged(QCPRange)), customPlot->yAxis2, SLOT(setRange(QCPRange)));

  // setup a timer that repeatedly calls MainWindow::realtimeDataSlot:
  connect(&dataTimer, SIGNAL(timeout()), this, SLOT(realtimeDataSlot()));
  dataTimer.start(0); // Interval 0 means to refresh as fast as possible
  //QFuture<void> future;
  //future = QtConcurrent::run (this,&MainWindow::/*directPowerSsvepDetection*/ParseSignalFile);
}

void MainWindow::setupFFTDataPlot(QCustomPlot *customPlot,double data)
{

  // add two new graphs and set their look:
  customPlot->addGraph();
  customPlot->graph(0)->setPen(QPen(Qt::blue)); // line color blue for first graph
  customPlot->graph(0)->setBrush(QBrush(QColor(0, 0, 255, 20))); // first graph will be filled with translucent blue
  customPlot->addGraph();
  customPlot->graph(1)->setPen(QPen(Qt::red)); // line color red for second graph
  // generate some points of data (y0 for first, y1 for second graph):
  QVector<double> x(N), y0(N), y1(N);

  //****************FFT****************************

  oldest_data = in[idx];
  newest_data = in[idx] = data;// */ double(rand() / double(N));
  sdft();

  // Mess about with freqs[] here
  //isdft();

  if (++idx == N) idx = 0;

  double powr1[N/2];
//  powr_spectrum(powr1);


   // bump global index
  //*********************************************
  for (int i=0; i<N; i++)
  {
    x[i] = i;
    y0[i] =freqs[i].real()+20; 
  }
  // configure right and top axis to show ticks but no labels:
  // (see QCPAxisRect::setupFullAxesBox for a quicker method to do this)
  customPlot->xAxis2->setVisible(true);
  customPlot->xAxis2->setTickLabels(false);
  customPlot->yAxis2->setVisible(true);
  customPlot->yAxis2->setTickLabels(false);
  // make left and bottom axes always transfer their ranges to right and top axes:
  connect(customPlot->xAxis, SIGNAL(rangeChanged(QCPRange)), customPlot->xAxis2, SLOT(setRange(QCPRange)));
  connect(customPlot->yAxis, SIGNAL(rangeChanged(QCPRange)), customPlot->yAxis2, SLOT(setRange(QCPRange)));

  // pass data points to graphs:
  customPlot->graph(0)->setData(x, y0);
  customPlot->graph(1)->setData(x, y1);
  // let the ranges scale themselves so graph 0 fits perfectly in the visible area:
  customPlot->graph(0)->rescaleAxes();
  // same thing for graph 1, but only enlarge ranges (in case graph 1 is smaller than graph 0):
  customPlot->graph(1)->rescaleAxes(true);
  // Note: we could have also just called customPlot->rescaleAxes(); instead
  // Allow user to drag axis ranges with mouse, zoom with mouse wheel and select graphs by clicking:
  customPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables);
}

void MainWindow::realtimeDataSlot()     //EDW EIMASTE
{
  // calculate two new data points:
#if QT_VERSION < QT_VERSION_CHECK(4, 7, 0)
  double key = 0;
#else
  double key = QDateTime::currentDateTime().toMSecsSinceEpoch()/500.0;
#endif

  //**********************************************************************
  static double lastPointKey = 0;
  double value0 = 0;
  double value1 = 0;
  if (key-lastPointKey > 0.0005) // at most add point every 10 ms
  {


    if  (ui->radioButton_randData->isChecked())
    {
        value0 = double(rand());
        serialDataList << value0;
    }
    else if (ui->radioButton_offline->isChecked() || ui->radioButton_online->isChecked())
        value0 = globalSerialDataValue;
        qDebug() << value0 << "hi";
    value1 = value0 ;

    // add data to lines:
    ui->customPlot->graph(0)->addData(key, value0);
    ui->customPlot->graph(1)->addData(key, value1);
    // set data of dots:
    ui->customPlot->graph(2)->clearData();
    ui->customPlot->graph(2)->addData(key, value0);
    ui->customPlot->graph(3)->clearData();
    ui->customPlot->graph(3)->addData(key, value1);
    // rescale value (vertical) axis to fit the current data:
    ui->customPlot->graph(0)->rescaleValueAxis(true);
    ui->customPlot->graph(1)->rescaleValueAxis(true);

    lastPointKey = key;
  }

  // make key axis range scroll with the data (at a constant range size of 8):
  ui->customPlot->xAxis->setRange(key+0.25, 8, Qt::AlignRight);
  ui->customPlot->replot();


  // calculate frames per second:
  static double lastFpsKey;
  static int frameCount;
  ++frameCount;
  if (key-lastFpsKey > 2) // average fps over 2 seconds
  {
//    ui->statusBar->showMessage(
//          QString("%1 FPS, Total Data points: %2")
//          .arg(frameCount/(key-lastFpsKey), 0, 'f', 0)
//          .arg(ui->customPlot->graph(0)->data()->count()+ui->customPlot->graph(1)->data()->count())
//          , 0);
    lastFpsKey = key;
    frameCount = 0;
  }

  // CAll FFT Plot And Update Values
  //**************************************************************
  setupFFTDataPlot(ui->customPlot_FFT,value0);
  statusBar()->clearMessage();
  ui->customPlot_FFT->replot();
  //**************************************************************
}


//********************Serial Communication Segment Start ****************************
//*************************************************************************************

void MainWindow::SendCommandToBoard(QString Data)
{
  if (port->isOpen() && !ui->sendLineEdit->text().isEmpty())
        port->write(ui->sendLineEdit->text().toLatin1());
    qDebug() << Data;
  port->flush();
}





//************************Serial port Handling Segment Begining********************
//*********************************************************************************


bool status = true;

void MainWindow::listSerialPorts()
{
    //! [1]
    QList<QextPortInfo> ports = QextSerialEnumerator::getPorts();
    //! [1]
    qDebug() << "List of ports:";
    //! [2]
    foreach (QextPortInfo info, ports) {
        qDebug() << "port name:"       << info.portName;
        qDebug() << "friendly name:"   << info.friendName;
        qDebug() << "physical name:"   << info.physName;
        qDebug() << "enumerator name:" << info.enumName;
        qDebug() << "vendor ID:"       << info.vendorID;
        qDebug() << "product ID:"      << info.productID;

        qDebug() << "===================================";
    }
    //! [2]
}
void MainWindow::onPortNameChanged(const QString & /*name*/)
{
    if (port->isOpen()) {
        port->close();
        //ui->led->turnOff();
    }
}
//! [2]
void MainWindow::onBaudRateChanged(int idx)
{
    port->setBaudRate((BaudRateType)ui->baudRateComboBox->itemData(idx).toInt());
}

void MainWindow::onParityChanged(int idx)
{
    port->setParity((ParityType)ui->parityComboBox->itemData(idx).toInt());
}

void MainWindow::onDataBitsChanged(int idx)
{
    port->setDataBits((DataBitsType)ui->dataBitComboBox->itemData(idx).toInt());
}

void MainWindow::onStopBitsChanged(int idx)
{
    port->setStopBits((StopBitsType)ui->StopbitComboBox->itemData(idx).toInt());
}

void MainWindow::onQueryModeChanged(int idx)
{
    port->setQueryMode((QextSerialPort::QueryMode)ui->queryModeComboBox->itemData(idx).toInt());
}

void MainWindow::onTimeoutChanged(int val)
{
    port->setTimeout(val);
}
//! [2]
//! [3]
void MainWindow::on_connectOrDisconnectBtn_clicked()
{
//    timer = new QTimer(this);
//    timer->setInterval(250);

    if (!port->isOpen()) {
        port->setPortName(ui->portComboBox->currentText());
        port->open(QIODevice::ReadWrite);

        if (port->isOpen())
        {
        ui->connectOrDisconnectBtn->setText("Disconnect");
        //ui->groupBox_->setEnabled(false);
        ui->groupBox_command->setEnabled(true);
        ui->ledIndicator_0->toggle();
        ui->ledIndicator_1->toggle();
        //ui->sendcommandBox->setEnabled(true);
        }
    }
    else {
        port->close();
        //port->flush();

        ui->connectOrDisconnectBtn->setText("Connect To Board");
        //ui->groupBox_->setEnabled(true);
        ui->groupBox_command->setEnabled(false);
        ui->ledIndicator_0->toggle();
        ui->ledIndicator_1->toggle();
        ui->ledIndicator_2->toggle();
        //ui->sendcommandBox->setEnabled(false);

    }

    //If using polling mode, we need a QTimer
    if (port->isOpen() && port->queryMode() == QextSerialPort::Polling)
        timer->start();
    else
        timer->stop();

    //update led's status
    //ui->led->turnOn(port->isOpen());

    //ui->led->turnOn(port->isOpen());




}



void MainWindow::on_SendToSerialPort_clicked()
{



        port->write(ui->sendLineEdit->text().toLatin1());
        ui->ledIndicator_2->toggle();
        ui->ledIndicator_2->setChecked(true);
        ui->sendLineEdit->clear();

    usleep(100000);
    ui->ledIndicator_2->toggle();
}


void MainWindow::onPortAddedOrRemoved()
{
    QString current = ui->portComboBox->currentText();

    ui->portComboBox->blockSignals(true);
    ui->portComboBox->clear();
    foreach (QextPortInfo info, QextSerialEnumerator::getPorts())
        ui->portComboBox->addItem(info.portName);

    ui->portComboBox->setCurrentIndex(ui->portComboBox->findText(current));

    ui->portComboBox->blockSignals(false);
}


void MainWindow::onReadyRead()
{

    if (port->bytesAvailable()){
   //    port->flush();
       ui->console->moveCursor(QTextCursor::End);
       QString recevedStr = QString::fromLatin1(port->readLine());
//       ui->console->insertPlainText(recevedStr);
       recevedStr=recevedStr.trimmed() ;
       QRegExp rx("(\\,)");
       QStringList query = recevedStr.split(rx);
       //read each tokens
       if(query.length()>=2){
       for (int i=0 ; i < query.length(); i++)
        {
            bool isNumeric = false;

            recevedStr = query.at(i);
            ui->console->insertPlainText(recevedStr+" ");
            qDebug() << recevedStr <<endl;

            /*int tempData = (recevedStr.toInt(&isNumeric)) ;

              if (isNumeric )
              {

                globalSerialDataValue=tempData;
                ui->console->insertPlainText(recevedStr);

                serialDataList <<globalSerialDataValue;*/

        //}
      }
      }
     ui->console->insertPlainText("\n");

     QString text = ui->sendLineEdit_3->text();  //patient's name
     qDebug() << text;
     QDate date = QDate::currentDate();
     QString dateString = date.toString();

     // Open csv-file
     for (int i=0 ; i < query.length()-1; i++)
    {
     QFile file(text + "-" + dateString+ ".csv");
     file.open(QIODevice::Append | QIODevice::Text);

     // Write data to file
     QTextStream stream(&file);
     QString separator(",");
     stream << query.at(i) << ";";
     stream.flush();
     file.close();
     }

     QFile file(text + "-" + dateString+ ".csv");
     file.open(QIODevice::Append | QIODevice::Text);
     // Write data to file
     QTextStream stream(&file);
     QString separator(",");
     stream << query.at(query.length()-1) << ";" <<endl;
     stream.flush();
     file.close();

   }
}

//************************File Handling Segment End********************************
//*********************************************************************************


void MainWindow::on_stopBtn_clicked()
{
    dataTimer.stop();
}



void MainWindow::on_commandLinkBtnClearConsole_clicked()
{
    ui->console->clear();
}


//************************FFT Calculation Segment begin*******************************
//************************************************************************************
#define PI  3.141592653589793238460

//initilaize e-to-the-i-thetas for theta = 0..2PI in increments of 1/N
void MainWindow::sdft_init_coeffs()
{
    for (int i = 0; i < N; ++i) {
        double a = -2.0 * PI * i  / double(N);
        coeffs[i] = Complex(cos(a)/* / N */, sin(a) /* / N */);
    }
    for (int i = 0; i < N; ++i) {
        double a = 2.0 * PI * i  / double(N);
        icoeffs[i] = Complex(cos(a),sin(a));
    }
}


// initialize all data buffers
void MainWindow::sdft_init()
{
    // clear data
    for (int i = 0; i < N; ++i)
        in[i] = 0.0;
//     seed rand()
//    srand(857); // for random number generation
    sdft_init_coeffs();
    oldest_data = newest_data = 0.0;
    idx = 0;
}

// simulating adding data to circular buffer
void MainWindow::sdft_add_data()
{

    oldest_data = in[idx];
    newest_data = in[idx] =0;// data;//  double(rand() / double(N));

}

// sliding dft
void MainWindow::sdft()
{
    double delta;
        delta = newest_data - oldest_data;
    int ci = 0;

    for (int i = 0; i < N; ++i)
    {
            //std::cout <<"ci ......"<<ci<<"\n";
            freqs[i] += delta * coeffs[ci];
//            if (freqs[i] >100)
//                freqs[i]=0;
            //std::cout << "\n sdtf Function  ....\n";
            //std::cout<<freqs[i][j];
            if ((ci += idx) >= N)
                ci -= N;
    }
}

double MainWindow::mag(Complex& c)
{
    return sqrt(c.real() * c.real() + c.imag() * c.imag());
}

void MainWindow::powr_spectrum(double *powr)
{
    for (int i = 0; i < N/2; ++i) {
        powr[i] = mag(freqs[i]);
    }

}
//************************FFT Calculation Segment End*******************************
//**********************************************************************************




//************************File Handling Segment Begin*******************************
//**********************************************************************************

#include <QFileDialog>

//global File Name
QString fileName="";
void MainWindow::on_toolButton_browse_clicked()
{
    fileName = QFileDialog::getOpenFileName(this, tr("Open File"),"/path/to/file/",tr(".txt Files (*.txt)"));
    ui->lineEdit_browse_file->setText(fileName);
    ParseSignalFile();
//    ui->listWidget->addItems(fileNames);
}


void MainWindow::ParseSignalFile(){

    if (fileName !="")
    {
        int tempgData;
        QFile file(fileName);
        //ui->statusBar->showMessage(filename);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text))
        {

            while (!file.atEnd())
            {
                QString line = file.readLine();
                QStringList query = line.split(",");
                for (int i=0 ; i < query.length(); i++)
                {
                    bool isNumeric = false;
                    QString tokenStr = query.at(i);
//                    ui->console->insertPlainText(tokenStr);
                    int tempgData = (tokenStr.toInt(&isNumeric));

                    if (isNumeric)
                    {
                        globalSerialDataValue=tempgData;
                        serialDataList<<globalSerialDataValue;
                    }
                }
            }
            file.close();
        }
    }
}

void MainWindow::logData(){

    qDebug()<<"eeee";
     //Log File Configuration
     //***********************************************
     QDateTime local(QDateTime::currentDateTime());
     qDebug() << "Local time is:" << local.toString();
     QFile logFile("LOG/LOG."+local.toString()+".log");

     if (logFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append))
     {
         QTextStream logSerialDataStream(&logFile);
         for (int i = 0; i < serialDataList.size(); ++i) {
             qDebug()<<serialDataList.at(i);
             logSerialDataStream << serialDataList.at(i)<<"\n";
          }
     }

     //***********************************************
}

//************************File Handling Segment End********************************
//*********************************************************************************




// MAIN CATEGORY LIST
//================================================================================================

//--------------------------------------------------------------------------------------
void MainWindow::on_mainAddButton_clicked()
{
    bool ok;
    QString text = QInputDialog::getText( this, tr("Add Category" ), tr( "Category Name:" ), QLineEdit::Normal, "", &ok );

    if ( ok && !text.isEmpty() )
    {
        // Make key
        int key = m_appStateData->listCategories->count() > 0 ? m_appStateData->listCategories->lastKey() + 1 : 0;

        // Add to category data list
        CategoryData *newCategory = new CategoryData();
        newCategory->name = text;
        m_appStateData->listCategories->insert( key, newCategory );

        // Add to UI list
        QListWidgetItem *newItem = new QListWidgetItem();
        newItem->setText( text );
        newItem->setData( Qt::UserRole, m_appStateData->listCategories->lastKey() );
        ui_mainCatListWidget->addItem( newItem );

        qDebug() << "Adding new category:" << text << "-" << m_appStateData->listCategories->lastKey();
    }
}


//--------------------------------------------------------------------------------------
void MainWindow::on_mainRemoveButton_clicked()
{
    if ( ui_mainCatListWidget->count() <= 0 )
        return;

    qDebug() << "on_mainRemoveButton_clicked():" << ui_mainCatListWidget->currentIndex().row();

    // Remove from app data
    QString removedCategory = ui_mainCatListWidget->currentItem()->text();
    QListWidgetItem* removedItem = ui_mainCatListWidget->currentItem();
    m_appStateData->listCategories->remove( removedItem->data( Qt::UserRole ).toInt() );

    // Find all images with that category and clear their category
    for ( int i = 0; i < m_appStateData->listImageData->count(); i++ )
    {
        ImageData* imgData = m_appStateData->listImageData->at( i );
        if ( imgData->category == removedCategory )
            imgData->category = "";
    }

    // Remove from UI list
    ui_mainCatListWidget->takeItem( ui_mainCatListWidget->currentRow() );

    // Disable subcategory buttons
    if ( ui_mainCatListWidget->count() <= 0 )
    {
        ui_subCatListWidget->clear();
        ui_subCatButtonLayout->setEnabled( false );
    }
}


//--------------------------------------------------------------------------------------
void MainWindow::on_mainCatListWidget_doubleClicked()
{
    // Set as editable
    QListWidgetItem *item = ui_mainCatListWidget->currentItem();
    item->setFlags( item->flags() | Qt::ItemIsEditable );
    ui_mainCatListWidget->editItem( ui_mainCatListWidget->currentItem() );
}


//--------------------------------------------------------------------------------------
void MainWindow::on_mainCatListWidget_itemChanged( QListWidgetItem *item )
{
    // Adjust name of corresponding entry in app_data
    CategoryData *catData = m_appStateData->listCategories->value( item->data( Qt::UserRole ).toInt() );

    // Check against old name
    QString oldName = catData->name;
    if ( oldName == item->text() )
        return;

    catData->name = item->text();

    // Find all images with this category and rename it accordingly
    for ( int i = 0; i < m_appStateData->listImageData->count(); i++ )
    {
        ImageData* imgData = m_appStateData->listImageData->at( i );
        if ( imgData->category == oldName )
            imgData->category = item->text();
    }

    qDebug() << "on_mainCatListWidget_itemChanged():" << oldName << "->" << catData->name;
}


//--------------------------------------------------------------------------------------
void MainWindow::on_mainCatListWidget_currentItemChanged( QListWidgetItem *current )
{
    if ( !current )
        return;

    // Update subcategory list
    ui_subCatListWidget->clear();
    CategoryData    *catData = m_appStateData->listCategories->value( current->data( Qt::UserRole ).toInt() );
    QList<QVariant> *subcats = m_appStateData->listCategories->value( current->data( Qt::UserRole ).toInt() )->subcategories;

    qDebug() << "on_mainCatListWidget_currentItemChanged:" << catData->name << "-" << current->data( Qt::UserRole ).toInt();

    // Populate subcategory ui list
    for ( int i = 0; i < subcats->length(); i++ )
    {
        QListWidgetItem *newItem = new QListWidgetItem();
        newItem->setText( subcats->at( i ).toString() );
        newItem->setData( Qt::UserRole, i );
        ui_subCatListWidget->addItem( newItem );
    }

    // Set current image's category
    if ( getCurrentImageData() )
        getCurrentImageData()->category = current->text();

    // Enable subcategory buttons
    ui_subCatButtonLayout->setEnabled( true );
}


// SUB-CATEGORY LIST
//================================================================================================

//--------------------------------------------------------------------------------------
void MainWindow::on_subAddButton_clicked()
{
    if ( ui_mainCatListWidget->currentRow() < 0 )
        return;

    bool ok;
    QString text = QInputDialog::getText( this, tr("Add Sub-Category" ), tr( "Sub-Category Name:" ), QLineEdit::Normal, "", &ok );

    if ( ok && !text.isEmpty() )
    {
        qDebug() << "Adding new sub-category:" << text;

        // Append to subcategory list
        QList<QVariant> *subcats = m_appStateData->listCategories->value( ui_mainCatListWidget->currentItem()->data( Qt::UserRole ).toInt() )->subcategories;
        subcats->append( QVariant( text ) );

        // Add new ui list item
        QListWidgetItem *newItem = new QListWidgetItem();
        newItem->setText( text );
        newItem->setData( Qt::UserRole, subcats->size() - 1 );
        ui_subCatListWidget->addItem( newItem );
    }
}


//--------------------------------------------------------------------------------------
void MainWindow::on_subRemoveButton_clicked()
{
    if ( ui_mainCatListWidget->currentRow() < 0 || ui_subCatListWidget->currentRow() < 0 )
        return;

    qDebug() << "on_subRemoveButton_clicked()";

    if ( ui_subCatListWidget->count() > 0 )
    {
        QString oldName = ui_subCatListWidget->currentItem()->text();

        // Remove from category data
        QList<QVariant> *subcats = m_appStateData->listCategories->value( ui_mainCatListWidget->currentItem()->data( Qt::UserRole ).toInt() )->subcategories;
        subcats->removeAt( subcats->indexOf( QVariant( ui_subCatListWidget->currentItem()->text() ) ) );

        // Find all images with that subcategory and blank it
        for ( int i = 0; i < m_appStateData->listImageData->count(); i++ )
        {
            ImageData* imgData = m_appStateData->listImageData->at( i );
            if ( imgData->subCategory == oldName )
                imgData->subCategory = "";
        }

        // Remove from subcategory ui list
        ui_subCatListWidget->takeItem( ui_subCatListWidget->currentIndex().row() );
    }
}


//--------------------------------------------------------------------------------------
void MainWindow::on_subCatListWidget_doubleClicked()
{
    // Set as editable
    QListWidgetItem *item = ui_subCatListWidget->currentItem();
    item->setFlags( item->flags() | Qt::ItemIsEditable );
    ui_subCatListWidget->editItem( ui_subCatListWidget->currentItem() );
}


//--------------------------------------------------------------------------------------
void MainWindow::on_subCatListWidget_itemChanged( QListWidgetItem *item )
{
    // Adjust name of corresponding entry in app_data
    CategoryData *catData       = m_appStateData->listCategories->value( ui_mainCatListWidget->currentItem()->data( Qt::UserRole ).toInt() );
    QList<QVariant> *subcatList = catData->subcategories;

    // Check against old name
    QString oldName = subcatList->at( item->data( Qt::UserRole ).toInt() ).toString();
    if ( oldName == item->text() )
        return;

    // Rename
    subcatList->replace( item->data( Qt::UserRole ).toInt(), QVariant( item->text() ) );

    // Find all images with this subcategory and rename it accordingly
    for ( int i = 0; i < m_appStateData->listImageData->count(); i++ )
    {
        ImageData* imgData = m_appStateData->listImageData->at( i );
        if ( imgData->subCategory == oldName )
            imgData->subCategory = item->text();
    }

    qDebug() << "on_subCatListWidget_itemChanged():" << oldName << "->" << subcatList->at( item->data( Qt::UserRole ).toInt() ).toString();
}


//--------------------------------------------------------------------------------------
void MainWindow::on_subCatListWidget_currentItemChanged( QListWidgetItem *current )
{
    if ( !current )
        return;

    qDebug() << "on_subCatListWidget_currentItemChanged(): " + current->text();

    if ( getCurrentImageData() )
        getCurrentImageData()->subCategory = current->text();
}



// MAIN FUNCTIONALITY
//================================================================================================

//--------------------------------------------------------------------------------------
void MainWindow::keyPressEvent( QKeyEvent *event )
{
    // Deal with delete keypress depending on what has focus
    if ( event->key() == Qt::Key_Delete || event->key() == Qt::Key_Backspace )
    {
        QWidget* focusWidget = QApplication::focusWidget();

        // Main category list
        if ( focusWidget == ui_mainCatListWidget )
        {
            on_mainRemoveButton_clicked();
            return;
        }

        // Sub-category list
        if ( focusWidget == ui_subCatListWidget )
        {
            on_subRemoveButton_clicked();
            return;
        }

        // File list
        if ( focusWidget == ui_fileListWidget )
        {
            if ( ui_fileListWidget->count() <= 0 )
                return;

            // Remove from UI list
            ui_fileListWidget->takeItem( ui_fileListWidget->currentRow() );
        }
    }
}

//--------------------------------------------------------------------------------------
void MainWindow::on_addFilesButton_clicked()
{
    qDebug() << "on_addFilesButton_clicked()";

    QFileDialog dialog( this );
    dialog.setFileMode( QFileDialog::ExistingFiles );
    dialog.setNameFilter( tr( "Images (*.png *.jpg *.bmp)" ) );
    dialog.setViewMode( QFileDialog::List );

    // Call dialog and process selected files
    QStringList fileNames;
    if ( dialog.exec() )
    {
        fileNames = dialog.selectedFiles();

        for ( int i = 0; i < fileNames.length(); i++ )
        {
            // Get file name and path
            QFileInfo *fileInfo = new QFileInfo( fileNames.at( i ) );
            QListWidgetItem *newItem = new QListWidgetItem( fileInfo->path(), ui_fileListWidget );

            // Create stock image data
            ImageData *newImageData = new ImageData();
            newImageData->path         = fileNames.at(i);
            newImageData->category     = tr( "" );
            newImageData->subCategory  = tr( "" );
            newImageData->gender       = tr( "Male" );
            newImageData->birthYear         = tr( "1960" );
            newImageData->size         = 1;
            newImageData->healthy       = false;
            newImageData->withBreathingDisease = false;
            newImageData->unknownMedicalIssue    = false;
            newImageData->microphoneModel     = tr( "CEM-C9745JAD462P2.54R" );
            newImageData->stretchSensorLength  = tr( "1" );

            m_appStateData->listImageData->append( newImageData );

            newItem->setData( Qt::UserRole, m_appStateData->listImageData->size() - 1 );
        }
    }
}


//--------------------------------------------------------------------------------------
void MainWindow::on_fileListWidget_itemSelectionChanged()
{
    ImageData *imgData = getCurrentImageData();

    // Reset if no image is found
    if ( !imgData )
    {
        ui_pathLabel->setText( tr("") );
        ui_mainCatListWidget->setCurrentRow( -1 );
        ui_subCatListWidget->setCurrentRow( -1 );
        ui_yearComboBox->setCurrentIndex(   ui_yearComboBox->findText("1970") );
        ui_colorComboBox->setCurrentIndex(  ui_colorComboBox->findText("Male") );
        ui_sizeSlider->setValue(            1 );
        ui_brokenCheckBox->setChecked(      false );
        ui_missingCheckBox->setChecked(     false );
        ui_batteriesCheckBox->setChecked(   false );
        ui_imagePreview->clear();
        return;
    }

    // Find and set category
    QList<QListWidgetItem *> itemCategories = ui_mainCatListWidget->findItems( imgData->category, Qt::MatchExactly );
    if ( itemCategories.length() > 0 )
    {
        ui_mainCatListWidget->setCurrentItem( itemCategories.first() );

        // Find and set sub-category
        QList<QListWidgetItem *> itemSubCategories = ui_subCatListWidget->findItems( imgData->subCategory, Qt::MatchExactly );
        if ( itemSubCategories.length() > 0 )
        {
            ui_subCatListWidget->setCurrentItem( itemSubCategories.first() );
        }
        else
        {
            // Clear selection
            QItemSelectionModel *selectionModel = ui_subCatListWidget->selectionModel();
            selectionModel->clear();
        }
    }
    else
    {
        // Clear main selection
        QItemSelectionModel * selectionModel = ui_mainCatListWidget->selectionModel();
        selectionModel->clear();
        ui_subCatListWidget->clear();
    }

    // Parse into the rest of the ui
    ui_pathLabel->setText(              "Path: " + imgData->path);
    ui_yearComboBox->setCurrentIndex( ui_yearComboBox->findText( imgData->birthYear ) );
    ui_colorComboBox->setCurrentIndex( ui_colorComboBox->findText( imgData->gender ) );
    ui_sizeSlider->setValue(            imgData->size );
    ui_brokenCheckBox->setChecked(      imgData->healthy );
    ui_missingCheckBox->setChecked(     imgData->withBreathingDisease );
    ui_batteriesCheckBox->setChecked(   imgData->unknownMedicalIssue );
    ui_imagePreview->setPixmap(         QPixmap( imgData->path ) );

    qDebug() << "on_fileListWidget_itemSelectionChanged():" << imgData->path;
}


//--------------------------------------------------------------------------------------
void MainWindow::on_saveButton_clicked()
{
    qDebug() << "on_saveButton_clicked()";
    //serializeAndSaveJsonFile( m_lastOpenMetafile );
}


//--------------------------------------------------------------------------------------
void MainWindow::on_saveAsButton_clicked()
{
    qDebug() << "on_saveAsButton_clicked()";

    // Prep file dialog
    QFileDialog dialog( this );
    dialog.setFileMode( QFileDialog::AnyFile );
    dialog.setNameFilter( tr( "Metadata File (*.meta)" ) );
    dialog.setViewMode( QFileDialog::List );
    dialog.setAcceptMode( QFileDialog::AcceptSave );

    if ( !dialog.exec() )
        return;

    serializeAndSaveJsonFile( dialog.selectedFiles().at( 0 ) );
}


//--------------------------------------------------------------------------------------
void MainWindow::on_loadButton_clicked()
{
    qDebug() << "on_loadButton_clicked()";

    // Prep file dialog
    QFileDialog dialog( this );
    dialog.setFileMode( QFileDialog::ExistingFile );
    dialog.setNameFilter( tr( "Metadata File (*.meta)" ) );
    dialog.setViewMode( QFileDialog::List );

    // Show dialog
    if ( !dialog.exec() )
        return;

    loadAndParseJsonFile( dialog.selectedFiles().at( 0 ) );
}


//--------------------------------------------------------------------------------------
void MainWindow::serializeAndSaveJsonFile( const QString path )
{
    // Serialize category data
    QJsonArray categoryJsonArray;
    QMap<int, CategoryData*>::const_iterator iter = m_appStateData->listCategories->constBegin();
    while ( iter != m_appStateData->listCategories->constEnd() )
    {
        categoryJsonArray.append( iter.value()->serializeToJson() );
        iter++;
    }

    // Serialize image data
    QJsonArray imageJsonArray;
    for ( int i = 0; i < m_appStateData->listImageData->count(); i++ )
        imageJsonArray.append( m_appStateData->listImageData->at( i )->serializeToJson() );

    // Put everything into a QJsonDocument
    QJsonObject metafileObj;
    metafileObj["categories"] = categoryJsonArray;
    metafileObj["images"] = imageJsonArray;
    QJsonDocument jsonDoc = QJsonDocument( metafileObj );

    // Write file
    QFile file;
    file.setFileName( path );
    file.open( QIODevice::WriteOnly | QIODevice::Text );
    file.write( jsonDoc.toJson() );

    qDebug() << "serializeAndSaveJsonFile() -" << path;

    // Save for later
    m_lastOpenMetafile = file.fileName();
    saveSettings();
}


//--------------------------------------------------------------------------------------
void MainWindow::loadAndParseJsonFile( const QString path )
{
    qDebug() << "loadAndParseJsonFile() -" << path;

    // Save for later
    m_loadedMetaFileName = path;

    // Load selected file
    QString jsonFromFile;
    QFile file;
    file.setFileName( path );
    file.open( QIODevice::ReadOnly | QIODevice::Text );
    jsonFromFile = file.readAll();
    file.close();

    // Get jsonObject out of jsonDoc
    QJsonDocument jsonDoc    = QJsonDocument::fromJson( jsonFromFile.toUtf8() );
    QJsonObject   jsonObject = jsonDoc.object();

    // Clear old app data
    ui_mainCatListWidget->clear();
    ui_subCatListWidget->clear();
    ui_fileListWidget->clear();
    m_appStateData->listCategories->clear();
    m_appStateData->listImageData->clear();

    // Parse categories
    QJsonArray categoriesArray = jsonObject.value( "categories" ).toArray();
    for ( int i = 0; i < categoriesArray.count(); i++ )
    {
        QJsonObject categoryObj = categoriesArray.at( i ).toObject();
        CategoryData *newCategoryData = new CategoryData();

        newCategoryData->name = categoryObj["name"].toString();

        // Add subcategories
        QList<QVariant> *subcats = new QList<QVariant>( categoryObj["subcategories"].toArray().toVariantList() );
        newCategoryData->subcategories = subcats;

        // Add to app data
        m_appStateData->listCategories->insert( i, newCategoryData ); //m_appStateData->listCategories->count(), newCategoryData );

        // Add categories to ui main category list
        QListWidgetItem *newItem = new QListWidgetItem();
        newItem->setText( categoryObj["name"].toString() );
        newItem->setData( Qt::UserRole, i );//m_appStateData->listCategories->count() - 1 );
        ui_mainCatListWidget->addItem( newItem );
    }

    // Parse images
    QJsonArray imagesArray = jsonObject.value( "images" ).toArray();
    for ( int i = 0; i < imagesArray.count(); i++ )
    {
        QJsonObject imageObj = imagesArray.at( i ).toObject();
        ImageData *newImageData = new ImageData();

        newImageData->path          = imageObj.value( "path" ).toString();
        newImageData->category      = imageObj.value( "category" ).toString();
        newImageData->subCategory   = imageObj.value( "subcategory" ).toString();
        newImageData->gender        = imageObj.value( "gender" ).toString();
        newImageData->birthYear     = imageObj.value( "birthYear" ).toString();
        newImageData->size          = (int)imageObj.value( "size" ).toDouble();
        newImageData->healthy        = imageObj.value( "healthy" ).toBool();
        newImageData->withBreathingDisease  = imageObj.value( "withBreathingDisease" ).toBool();
        newImageData->unknownMedicalIssue     = imageObj.value( "unknownMedicalIssue" ).toBool();
        newImageData->microphoneModel          = imageObj.value( "microphoneModel" ).toString();               //REPAIR
        newImageData->stretchSensorLength   = imageObj.value( "stretchSensorLength" ).toString();

        // Add to app data
        m_appStateData->listImageData->append( newImageData );

        // Add to file list ui
        QListWidgetItem *newItem = new QListWidgetItem();
        newItem->setText( newImageData->path );
        newItem->setData( Qt::UserRole, m_appStateData->listImageData->count() - 1);
        ui_fileListWidget->addItem( newItem );
    }

    // Enable save button
    ui_saveButton->setEnabled( true );

    // TODO: Set title bar to show meta file name

    saveSettings();
}


//--------------------------------------------------------------------------------------
void MainWindow::saveSettings()
{
    qDebug() << "saveSettings()";
    QSettings settings1( "Jeremy Abel", "Spectra" );
    settings1.setValue( "lastMetaFile", m_lastOpenMetafile );
}


//--------------------------------------------------------------------------------------
void MainWindow::on_colorComboBox_currentIndexChanged()     { if ( getCurrentImageData() ) getCurrentImageData()->gender          = ui_colorComboBox->currentText(); }
void MainWindow::on_yearComboBox_currentIndexChanged()      { if ( getCurrentImageData() ) getCurrentImageData()->birthYear           = ui_yearComboBox->currentText(); }
void MainWindow::on_brokenCheckBox_toggled(bool checked)    { if ( getCurrentImageData() ) getCurrentImageData()->healthy         = checked; }
void MainWindow::on_missingCheckBox_toggled(bool checked)   { if ( getCurrentImageData() ) getCurrentImageData()->withBreathingDisease   = checked; }
void MainWindow::on_batteriesCheckBox_toggled(bool checked) { if ( getCurrentImageData() ) getCurrentImageData()->unknownMedicalIssue      = checked; }


//--------------------------------------------------------------------------------------
void MainWindow::on_sizeSlider_valueChanged( int value )
{
    switch ( value )
    {
        case 0: ui_sizeLabel->setText( tr( "Small" ) );     break;
        case 1: ui_sizeLabel->setText( tr( "Medium" ) );    break;
        case 2: ui_sizeLabel->setText( tr( "Large" ) );     break;
    }

    if ( getCurrentImageData() )
        getCurrentImageData()->size = value;
}


//--------------------------------------------------------------------------------------
ImageData* MainWindow::getCurrentImageData()
{
    if ( !ui_fileListWidget->currentItem() )
        return NULL;

    return m_appStateData->listImageData->at( ui_fileListWidget->currentItem()->data( Qt::UserRole ).toInt() );
}





void MainWindow::on_comboBox_currentIndexChanged(int index)
{
    switch ( index )
    {
    case 0: ui->sensor1->setText("");
            ui->sensor2->setText("");
            ui->sensor3->setText("");
            break;
    case 1: ui->sensor1->setText("Sensor1");
            ui->sensor2->setText("");
            ui->sensor3->setText("");
            break;
    case 2: ui->sensor1->setText("Sensor1");
            ui->sensor2->setText("Sensor2");
            ui->sensor3->setText("");
            break;
    case 3: ui->sensor1->setText("Sensor1");
            ui->sensor2->setText("Sensor2");
            ui->sensor3->setText("Sensor3");
            break;
    }
}


void MainWindow::on_comboBox_3_currentIndexChanged(int index)
{
    switch ( index )
    {
    case 0: ui->sensor1->setText("");
            ui->sensor2->setText("");
            ui->sensor3->setText("");
            break;
    case 1: ui->sensor1->setText("Sensor1");
            ui->sensor2->setText("");
            ui->sensor3->setText("");
            break;
    case 2: ui->sensor1->setText("Sensor1");
            ui->sensor2->setText("Sensor2");
            ui->sensor3->setText("");
            break;
    case 3: ui->sensor1->setText("Sensor1");
            ui->sensor2->setText("Sensor2");
            ui->sensor3->setText("Sensor3");
            break;
    }
}


void MainWindow::on_comboBox_4_currentIndexChanged(int index)
{
    switch ( index )
    {
    case 0: ui->sensor1->setText("");
            ui->sensor2->setText("");
            ui->sensor3->setText("");
            break;
    case 1: ui->sensor1->setText("Sensor1");
            ui->sensor2->setText("");
            ui->sensor3->setText("");
            break;
    case 2: ui->sensor1->setText("Sensor1");
            ui->sensor2->setText("Sensor2");
            ui->sensor3->setText("");
            break;
    case 3: ui->sensor1->setText("Sensor1");
            ui->sensor2->setText("Sensor2");
            ui->sensor3->setText("Sensor3");
            break;
    }
}


void MainWindow::on_comboBox_5_currentIndexChanged(int index)
{
    switch ( index )
    {
    case 0: ui->sensor1->setText("");
            ui->sensor2->setText("");
            ui->sensor3->setText("");
            break;
    case 1: ui->sensor1->setText("Sensor1");
            ui->sensor2->setText("");
            ui->sensor3->setText("");
            break;
    case 2: ui->sensor1->setText("Sensor1");
            ui->sensor2->setText("Sensor2");
            ui->sensor3->setText("");
            break;
    case 3: ui->sensor1->setText("Sensor1");
            ui->sensor2->setText("Sensor2");
            ui->sensor3->setText("Sensor3");
            break;
           // ui_imagePreview->setPixmap(         QPixmap( imgData->path ) );
    }
}
