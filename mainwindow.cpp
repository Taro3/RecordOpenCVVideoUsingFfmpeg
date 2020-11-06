#include <QProcess>
#include <QDebug>

#include <opencv2/opencv.hpp>

#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_clicked()
{
    cv::VideoCapture vc(0);
    if (!vc.isOpened())
        return;

    QProcess p;
#if !defined(USE_RAWVIDEO)
    // record mp4(mjpeg to h264)
    QStringList args = {"-y", "-f", "image2pipe", "-vcodec", "mjpeg", "-r", "24", "-i", "-", "-vcodec", "h264"
                        , "-qscale", "5", "-r", "24", "video.mp4",};
#else
    // record mp4(rawvideo to h264)
    QStringList args = {"-y", "-f", "rawvideo", "-pix_fmt", "bgr24", "-s", "640x480",  "-r", "24", "-i", "-", "-vcodec"
                        , "h264", "-qscale", "5", "-r", "24", "video.mp4",};
#endif
    p.setProgram("ffmpeg");
    p.setArguments(args);
    p.start();
    p.waitForStarted();
    if (!p.isOpen())
        return;
    qDebug() << "start";
    while (true) {
        if (cv::waitKey(1) >= 0)
            break;
        cv::Mat frame;
        vc >> frame;
        if (frame.empty())
            break;
        cv::imshow("Video", frame);
#if !defined(USE_RAWVIDEO)
        // convert Mat to jpeg
        cv::Mat outFrame;
        cv::cvtColor(frame, outFrame, cv::COLOR_BGR2RGB);
        QImage image(outFrame.data, outFrame.cols, outFrame.rows, outFrame.step, QImage::Format_RGB888);
        // write jpeg via sidin
        image.save(&p, "jpg");
#else
        // write rawvideo via sitin
        p.write((char*)frame.data, frame.cols * frame.rows * frame.channels());
#endif
    }
    p.waitForBytesWritten();
    p.closeWriteChannel();
    p.waitForFinished();
    p.close();
    vc.release();
    cv::destroyAllWindows();
    qDebug() << "finish";
}
