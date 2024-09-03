#ifndef GUIPANEL_H
#define GUIPANEL_H

#include <QWidget>
#include <QtSerialPort/qserialport.h>
#include "qmqtt.h"

namespace Ui {
class GUIPanel;
}

//QT4:QT_USE_NAMESPACE_SERIALPORT

class GUIPanel : public QWidget
{
    Q_OBJECT
    
public:
    //GUIPanel(QWidget *parent = 0);
    explicit GUIPanel(QWidget *parent = 0);
    ~GUIPanel(); // Da problemas
    
private slots:

    void on_runButton_clicked();
    void on_pushButton_clicked();

    void onMQTT_Received(const QMQTT::Message &message);
    void onMQTT_Connected(void);

    void onMQTT_subscribed(const QString &topic);
    void on_pingButton_clicked(); // FVL declaracion funcion ping
    //void on_sondeoButton_clicked(); // FVL declaracion funcion
    //void on_modo_currentIndexChanged(int index);


    void on_pushButton_2_toggled(bool checked);

    void on_pushButton_4_toggled(bool checked);

    void on_pushButton_3_toggled(bool checked);

    void on_pushButton_temp_toggled(bool checked);

    void on_pushButton_bme_toggled(bool checked);

    void on_pushButton_adc_toggled(bool checked);

    //void on_Knob_Rojo_valueChanged();

    //void on_Knob_Verde_valueChanged();

    //void on_Knob_Azul_valueChanged();

    //void on_async_toggled();

    void on_tempslider_sliderReleased();

    void on_bmeslider_sliderReleased();

    void on_habilitar_alarma_toggled();

    void on_umbral_alarma_sliderReleased();

    void on_umbral_alarma_bme_sliderReleased();

    void on_habilitar_alarma_bme_toggled();


private: // funciones privadas
//    void pingDevice();
    void startClient();
    void processError(const QString &s);
    void activateRunButton();
    void cambiaLEDs();
    void SendMessage();
private:
    Ui::GUIPanel *ui;
    int transactionCount;
    QMQTT::Client *_client;
    bool connected;
};

#endif // GUIPANEL_H
