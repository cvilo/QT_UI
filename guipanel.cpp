#include "guipanel.h"
#include "ui_guipanel.h"
#include <QSerialPort>      // Comunicacion por el puerto serie
#include <QSerialPortInfo>  // Comunicacion por el puerto serie
#include <QMessageBox>      // Se deben incluir cabeceras a los componentes que se vayan a crear en la clase

#include <QJsonObject>
#include <QJsonDocument>

// y que no estén incluidos en el interfaz gráfico. En este caso, la ventana de PopUp <QMessageBox>
// que se muestra al recibir un PING de respuesta

#include<stdint.h>      // Cabecera para usar tipos de enteros con tamaño
#include<stdbool.h>     // Cabecera para usar booleanos


GUIPanel::GUIPanel(QWidget *parent) :  // Constructor de la clase
    QWidget(parent),
    ui(new Ui::GUIPanel)               // Indica que guipanel.ui es el interfaz grafico de la clase
  , transactionCount(0)
{
    ui->setupUi(this);                // Conecta la clase con su interfaz gráfico.
    setWindowTitle(tr("Interfaz de Control")); // Título de la ventana

    _client=new QMQTT::Client(QHostAddress::LocalHost, 1883); //localhost y lo otro son valores por defecto


    connect(_client, SIGNAL(connected()), this, SLOT(onMQTT_Connected()));    
    connect(_client, SIGNAL(received(const QMQTT::Message &)), this, SLOT(onMQTT_Received(const QMQTT::Message &)));
    connect(_client, SIGNAL(subscribed(const QString &)), this, SLOT(onMQTT_subscribed(const QString &)));    

    connected=false;                 // Todavía no hemos establecido la conexión USB

    //FVL Deshabilita los botones hasta que se establezca la conexion

    ui->pingButton->setEnabled(false);
    //ui->modo->setEnabled(false);
    //ui->DAC_voltage->setEnabled(false);//Solución EF44
    //ui->Set_Time->setEnabled(false);//Solución EF45
    //ui->rojo->setEnabled(false);
    //ui->verde->setEnabled(false);
    //ui->azul->setEnabled(false);
    //ui->async->setEnabled(false);

    //ui->start->setEnabled(false);
}

GUIPanel::~GUIPanel() // Destructor de la clase
{
    delete ui;   // Borra el interfaz gráfico asociado a la clase
}


// Establecimiento de la comunicación USB serie a través del interfaz seleccionado en la comboBox, tras pulsar el
// botón RUN del interfaz gráfico. Se establece una comunicacion a 9600bps 8N1 y sin control de flujo en el objeto
// 'serial' que es el que gestiona la comunicación USB serie en el interfaz QT
void GUIPanel::startClient()
{
    _client->setHostName(ui->leHost->text());
    _client->setPort(1883);
    _client->setKeepAlive(300);
    _client->setCleanSession(true);
    _client->connectToHost();

}

// Funcion auxiliar de procesamiento de errores de comunicación (usada por startSlave)
void GUIPanel::processError(const QString &s)
{
    activateRunButton(); // Activa el botón RUN
    // Muestra en la etiqueta de estado la razón del error (notese la forma de pasar argumentos a la cadena de texto)
    ui->statusLabel->setText(tr("Status: Not running, %1.").arg(s));
}

// Funcion de habilitacion del boton de inicio/conexion
void GUIPanel::activateRunButton()
{
    ui->runButton->setEnabled(true);
}

// SLOT asociada a pulsación del botón RUN
void GUIPanel::on_runButton_clicked()
{
    startClient();
}

void GUIPanel::onMQTT_subscribed(const QString &topic)
{
     ui->statusLabel->setText(tr("subscribed %1").arg(topic));
}


void GUIPanel::on_pushButton_clicked()
{
    ui->statusLabel->setText(tr(""));
}


void GUIPanel::onMQTT_Received(const QMQTT::Message &message)
{

    bool previousblockinstate,checked;
    if (connected)
    {
        //Deshacemos el escalado

        QJsonParseError error;
        QJsonDocument mensaje=QJsonDocument::fromJson(message.payload(),&error);

        if ((error.error==QJsonParseError::NoError)&&(mensaje.isObject()))
        { //Tengo que comprobar que el mensaje es del tipo adecuado y no hay errores de parseo...

            QJsonObject objeto_json=mensaje.object();

            if (message.topic()=="/mseei/ambisense/fvl/CTR"){

                QJsonValue entrada=objeto_json["redLed"]; //Obtengo la entrada redLed. Esto lo puedo hacer porque el operador [] está sobrecargado
                QJsonValue entrada2=objeto_json["greenLed"]; //Obtengo la entrada orangeLed. Esto lo puedo hacer porque el operador [] está sobrecargado
                QJsonValue entrada3=objeto_json["blueLed"]; //Obtengo la entrada greenLed. Esto lo puedo hacer porque el operador [] está sobrecargado
                //QJsonValue entrada_modo=objeto_json["modo"]; //Obtengo la entrada greenLed. Esto lo puedo hacer porque el operador [] está sobrecargado
                //QJsonValue entrada_pwm_rojo=objeto_json["pwm_rojo"]; //Obtengo la entrada greenLed. Esto lo puedo hacer porque el operador [] está sobrecargado
                //QJsonValue entrada_pwm_verde=objeto_json["pwm_verde"]; //Obtengo la entrada greenLed. Esto lo puedo hacer porque el operador [] está sobrecargado
                //QJsonValue entrada_pwm_azul=objeto_json["pwm_azul"]; //Obtengo la entrada greenLed. Esto lo puedo hacer porque el operador [] está sobrecargado
                QJsonValue entrada_estado_temp=objeto_json["sensor_temp"];
                QJsonValue entrada_estado_adc=objeto_json["sensor_adc"];
                QJsonValue entrada_temp_slider=objeto_json["sensor_temp_slider"];
                if (entrada.isBool())
                {   //Compruebo que es booleano...

                    checked=entrada.toBool(); //Leo el valor de objeto (si fuese entero usaria toInt(), toDouble() si es doble....
                    previousblockinstate=ui->pushButton_2->blockSignals(true);   //Esto es para evitar que el cambio de valor
                                                                         //provoque otro envio al topic por el que he recibido

                    ui->pushButton_2->setChecked(checked);
                    if (checked)
                    {
                        ui->pushButton_2->setText("Apaga");
                    }
                    else
                    {
                        ui->pushButton_2->setText("Enciende");
                    }
                    ui->pushButton_2->blockSignals(previousblockinstate);
                }
                if (entrada2.isBool())
                {   //Compruebo que es booleano...

                    checked=entrada2.toBool(); //Leo el valor de objeto (si fuese entero usaria toInt(), toDouble() si es doble....
                    previousblockinstate=ui->pushButton_3->blockSignals(true);   //Esto es para evitar que el cambio de valor
                                                                         //provoque otro envio al topic por el que he recibido
                    ui->pushButton_3->setChecked(checked);
                    if (checked)
                    {
                        ui->pushButton_3->setText("Apaga");
                    }
                    else
                    {
                        ui->pushButton_3->setText("Enciende");
                    }
                    ui->pushButton_3->blockSignals(previousblockinstate);
                }
                if (entrada3.isBool())
                {   //Compruebo que es booleano...

                    checked=entrada3.toBool(); //Leo el valor de objeto (si fuese entero usaria toInt(), toDouble() si es doble....
                    previousblockinstate=ui->pushButton_4->blockSignals(true);   //Esto es para evitar que el cambio de valor
                                                                         //provoque otro envio al topic por el que he recibido
                    ui->pushButton_4->setChecked(checked);
                    if (checked)
                    {
                        ui->pushButton_4->setText("Apaga");
                    }
                    else
                    {
                        ui->pushButton_4->setText("Enciende");
                    }
                    ui->pushButton_4->blockSignals(previousblockinstate);
                }
                if (entrada_estado_adc.isBool())
                {   //Compruebo que es booleano...

                    checked=entrada_estado_adc.toBool(); //Leo el valor de objeto (si fuese entero usaria toInt(), toDouble() si es doble....
                    previousblockinstate=ui->pushButton_adc->blockSignals(true);   //Esto es para evitar que el cambio de valor
                                                                         //provoque otro envio al topic por el que he recibido

                    ui->pushButton_adc->setChecked(checked);
                    if (checked)
                    {
                        ui->pushButton_adc->setText("Apaga");

                    }
                    else
                    {
                        ui->pushButton_adc->setText("Enciende");
                    }
                    ui->pushButton_adc->blockSignals(previousblockinstate);
                }
                if (entrada_estado_temp.isBool())
                {   //Compruebo que es booleano...

                    checked=entrada_estado_temp.toBool(); //Leo el valor de objeto (si fuese entero usaria toInt(), toDouble() si es doble....
                    previousblockinstate=ui->pushButton_temp->blockSignals(true);   //Esto es para evitar que el cambio de valor
                                                                         //provoque otro envio al topic por el que he recibido

                    ui->pushButton_temp->setChecked(checked);
                    if (checked)
                    {
                        ui->pushButton_temp->setText("Apaga");
                            previousblockinstate=ui->tempslider->blockSignals(true);
                            ui->tempslider->setValue((entrada_temp_slider).toDouble());
                            ui->tempslider->blockSignals(previousblockinstate);

                    }
                    else
                    {
                        ui->pushButton_temp->setText("Enciende");
                    }
                    ui->pushButton_temp->blockSignals(previousblockinstate);
                }


            }
            else if(message.topic()=="/mseei/ambisense/fvl/PING"){
                QJsonValue entrada=objeto_json["ping"]; //Obtengo la entrada redLed. Esto lo puedo hacer porque el operador [] está sobrecargado


                if (entrada.isBool())
                {   //Compruebo que es booleano...

                    if (ui->pingButton->property("waiting_ping").toBool())
                    {
                        ui->pingButton->setProperty("waiting_ping", false);
                        QMessageBox ventanaPopUp(QMessageBox::Information,tr("Evento"),tr("RESPUESTA A PING RECIBIDA"),QMessageBox::Ok,this,Qt::Popup);
                        ventanaPopUp.exec();

                    }
                    else
                    {

                    }
                }
            }
//            else if(message.topic()=="/mseei/ambisense/fvl/SOND"){
//                QJsonValue estado_boton_1=objeto_json["button_1"]; //Obtengo la entrada redLed. Esto lo puedo hacer porque el operador [] está sobrecargado
//                QJsonValue estado_boton_2=objeto_json["button_2"]; //Obtengo la entrada redLed. Esto lo puedo hacer porque el operador [] está sobrecargado
//                ui->sondeoButton->setProperty("sondeo", false);
//                if (estado_boton_1.isBool())
//                {   //Compruebo que es booleano...
//                    if (estado_boton_1 == true){
//                        ui->led_1->setColor(Qt::green);}
//                    else{
//                        ui->led_1->setColor(Qt::red);}
//                    }

//                if (estado_boton_2.isBool())
//                {   //Compruebo que es booleano...
//                    if (estado_boton_2 == true){
//                        ui->led_2->setColor(Qt::green);
//                    }
//                    else{
//                        ui->led_2->setColor(Qt::red);
//                    }
//                }
//            }
            else if(message.topic()=="/mseei/ambisense/fvl/POTENC"){
                QJsonValue valor_potenciometro=objeto_json["adc_potenc"]; //Obtengo la entrada redLed. Esto lo puedo hacer porque el operador [] está sobrecargado

                if (valor_potenciometro.isDouble())
                {   //Compruebo que es booleano...

                    ui->potenciometro->setValue(valor_potenciometro.toDouble());

                }
            }
            else if(message.topic()=="/mseei/ambisense/fvl/STATUS"){
                QJsonValue board_status=objeto_json["estado"]; //Obtengo la entrada redLed. Esto lo puedo hacer porque el operador [] está sobrecargado

                if (board_status.isString())
                {   //Compruebo que es booleano...

                    ui->board_status->setText(board_status.toString());

                }
            }
            else if(message.topic()=="/mseei/ambisense/fvl/TEMP"){
                QJsonValue entrada_temp=objeto_json["temp"];

                if(entrada_temp.isDouble()){
                        ui->lcd_temp->display((entrada_temp).toDouble());
                }
            }
            else if(message.topic()=="/mseei/ambisense/fvl/BME_SENSOR"){
                QJsonValue entrada_temp=objeto_json["temp_bme680"];
                QJsonValue entrada_press=objeto_json["press_bme680"];
                QJsonValue entrada_hum=objeto_json["hum_bme680"];
                QJsonValue entrada_gas=objeto_json["gas_iaq"];
                QString IAQ_text = "unstable";
                if(entrada_temp.isDouble()){
                        ui->temp_ind->setValue((entrada_temp).toDouble());
                }
                if(entrada_press.isDouble()){
                        ui->pression_ind->setValue((entrada_press).toDouble());
                }
                if(entrada_hum.isDouble()){
                        ui->humedad_ind->setValue((entrada_hum).toDouble());
                }
                if(entrada_gas.isDouble()){
                        ui->gas_ind->display((entrada_gas).toDouble());
                        double gas_value = entrada_gas.toDouble();
                        if    (gas_value >= 301)                      IAQ_text = "Hazardous";
                      else if (gas_value >= 201 && gas_value <= 300)  IAQ_text = "Very Unhealthy";
                      else if (gas_value >= 176 && gas_value <= 200)  IAQ_text = "Unhealthy";
                      else if (gas_value >= 151 && gas_value <= 175)  IAQ_text = "Unhealthy for Sensitive Groups";
                      else if (gas_value >=  51 && gas_value <= 150)  IAQ_text = "Moderate";
                      else if (gas_value >=   0 && gas_value <=  50)  IAQ_text = "Good";
                      ui->gas_status->setText(IAQ_text);
                }
            }
            else if(message.topic()=="/mseei/ambisense/fvl/ALARMA"){
                QJsonValue entrada_alarma=objeto_json["alarma_activada"];
                bool alarm_checked;
                if(entrada_alarma.isBool()){
                    alarm_checked=entrada_alarma.toBool();
                    if (alarm_checked == true){
                        ui->led_alarma->setColor(Qt::red);
                        ui->text_alarma_adc->setText("WANING");
                    }
                    else{
                        ui->led_alarma->setColor(Qt::green);
                        ui->text_alarma_adc->setText("SAFETY");
                    }
                }
            }

            else if(message.topic()=="/mseei/ambisense/fvl/ALARMA_BME"){
                QJsonValue entrada_alarma=objeto_json["alarma_activada_bme"];
                bool alarm_checked;
                if(entrada_alarma.isBool()){
                    alarm_checked=entrada_alarma.toBool();
                    if (alarm_checked == true){
                        ui->led_alarma_bme->setColor(Qt::red);
                        ui->text_alarma_bme->setText("WARNING ZONE");
                    }
                    else{
                        ui->led_alarma_bme->setColor(Qt::green);
                        ui->text_alarma_bme->setText("SAFETY ZONE");
                    }
                }
            }

        }
    }
}


/* -----------------------------------------------------------
 MQTT Client Slots
 -----------------------------------------------------------*/
void GUIPanel::onMQTT_Connected()
{
    //QString topic(ui->topic->text());
    QString topic_control = "/mseei/ambisense/fvl/CTR";
    QString topic_ping = "/mseei/ambisense/fvl/PING";
    QString topic_sondeo = "/mseei/ambisense/fvl/SOND";
    QString topic_potenc = "/mseei/ambisense/fvl/POTENC";
    QString topic_last_will = "/mseei/ambisense/fvl/STATUS";
    QString topic_temp = "/mseei/ambisense/fvl/TEMP";
    QString topic_bme = "/mseei/ambisense/fvl/BME_SENSOR";
    QString topic_alarma = "/mseei/ambisense/fvl/ALARMA";
    QString topic_alarma_bme = "/mseei/ambisense/fvl/ALARMA_BME";

    ui->runButton->setEnabled(false);

    // Se indica que se ha realizado la conexión en la etiqueta 'statusLabel'

    connected=true;
    //FVL Habilitamos boton de ping
    ui->pingButton->setEnabled(true);
    //ui->sondeoButton->setEnabled(true);
    ui->pingButton->setEnabled(true);
    //ui->sondeoButton->setEnabled(true);
    //ui->modo->setEnabled(true);
    //ui->DAC_voltage->setEnabled(true);//Solución EF44
    //ui->Set_Time->setEnabled(true);//Solución EF45
    //ui->rojo->setEnabled(true);
    //ui->verde->setEnabled(true);
    //ui->azul->setEnabled(true);

    _client->subscribe(topic_control,0); //Se suscribe al topic de control
    _client->subscribe(topic_ping,0); //Se suscribe al topic de ping
    //_client->subscribe(topic_sondeo,0); //Se suscribe al topic de sondeo
    _client->subscribe(topic_potenc,0); //Se suscribe al topic de potenciometro
    _client->subscribe(topic_last_will,0); //Se suscribe al topic de potenciometro
    _client->subscribe(topic_temp,0); //Se suscribe al topic de potenciometro
    _client->subscribe(topic_bme,0); //Se suscribe al topic de potenciometro
    _client->subscribe(topic_alarma,0); //Se suscribe al topic de potenciometro
    _client->subscribe(topic_alarma_bme,0); //Se suscribe al topic de potenciometro

}



void GUIPanel::SendMessage()
{

    QString topic_control = "/mseei/ambisense/fvl/CTR";

    QJsonObject objeto_json;
    //Añade un campo "redLed" al objeto JSON, con el valor (true o false) contenido en checked
    objeto_json["redLed"]=ui->pushButton_2->isChecked(); //Puedo hacer ["redLed"] porque el operador [] está sobrecargado.
    //Añade un campo "orangeLed" al objeto JSON, con el valor (true o false) contenido en checked
    objeto_json["greenLed"]=ui->pushButton_3->isChecked(); //Puedo hacer ["orangeLed"] porque el operador [] está sobrecargado.
    //Añade un campo "greenLed" al objeto JSON, con el valor (true o false) contenido en checked
    objeto_json["blueLed"]=ui->pushButton_4->isChecked();

    objeto_json["ping"]=ui->pingButton->property("waiting_ping").toBool(); //FVL añadimos un campo en caso de que queramos comprobar el ping
    //objeto_json["sondeo"]=ui->sondeoButton->property("sondeo").toBool(); //FVL añadimos un campo en caso de que queramos realizar el sondeo

    //objeto_json["modo"]=ui->modo->currentIndex();
    //objeto_json["async"]=ui->async->isChecked();

    objeto_json["sensor_adc"]=ui->pushButton_adc->isChecked();
    objeto_json["sensor_temp"]=ui->pushButton_temp->isChecked();
    objeto_json["sensor_bme"]=ui->pushButton_bme->isChecked();

    objeto_json["sensor_temp_slider"]=(uint16_t) (ui->tempslider->value());
    objeto_json["sensor_bme_slider"]=(uint16_t) (ui->bmeslider->value());

    objeto_json["habilitar_alarma"]=ui->habilitar_alarma->isChecked();
    objeto_json["umbral_alarma"]=(uint16_t) (ui->umbral_alarma->value());

    objeto_json["habilitar_alarma_bme"]=ui->habilitar_alarma_bme->isChecked();
    objeto_json["umbral_alarma_bme"]=(uint16_t) (ui->umbral_alarma_bme->value());

    QJsonDocument mensaje(objeto_json); //crea un objeto de tivo QJsonDocument conteniendo el objeto objeto_json (necesario para obtener el mensaje formateado en JSON)
    QMQTT::Message msg(0, topic_control, mensaje.toJson()); //Crea el mensaje MQTT contieniendo el mensaje en formato JSON

    //Publica el mensaje
    _client->publish(msg);

}

//FVL añadimos funcionalidad al boton de ping
void GUIPanel::on_pingButton_clicked()
{
    ui->pingButton->setProperty("waiting_ping", true);
    SendMessage();
}

//FVL añadimos funcionalidad al boton de ping
//void GUIPanel::on_sondeoButton_clicked()
//{
//    ui->sondeoButton->setProperty("sondeo", true);
//    SendMessage();
//}

void GUIPanel::on_pushButton_2_toggled(bool checked)
{
    //Rojo
    if (checked)
    {
        ui->pushButton_2->setText("Apaga");
    }
    else
    {
        ui->pushButton_2->setText("Enciende");
    }
    SendMessage();
}
void GUIPanel::on_pushButton_3_toggled(bool checked)
{
    if (checked)
    {
        ui->pushButton_3->setText("Apaga");
    }
    else
    {
        ui->pushButton_3->setText("Enciende");
    }
    SendMessage();
}
void GUIPanel::on_pushButton_4_toggled(bool checked)
{
    //Verde
    if (checked)
    {
        ui->pushButton_4->setText("Apaga");
    }
    else
    {
        ui->pushButton_4->setText("Enciende");
    }
    SendMessage();
}

void GUIPanel::on_pushButton_temp_toggled(bool checked)
{
    //Verde
    if (checked)
    {
        ui->pushButton_temp->setText("Apaga");
    }
    else
    {
        ui->pushButton_temp->setText("Enciende");
    }
    SendMessage();
}
void GUIPanel::on_pushButton_bme_toggled(bool checked)
{
    //Verde
    if (checked)
    {
        ui->pushButton_bme->setText("Apaga");
    }
    else
    {
        ui->pushButton_bme->setText("Enciende");
    }
    SendMessage();
}
void GUIPanel::on_pushButton_adc_toggled(bool checked)
{
    //Verde
    if (checked)
    {
        ui->pushButton_adc->setText("Apaga");
    }
    else
    {
        ui->pushButton_adc->setText("Enciende");
    }
    SendMessage();
}
//void GUIPanel::on_modo_currentIndexChanged(int index)
//{
//            switch (index)
//            {
//                case 0:
//                    break;
//                case 1:
//                    break;
//            }
//    SendMessage();
//}


////Envia los PWM de las roscas
//void GUIPanel::on_Knob_Rojo_valueChanged()
//{

//    if (connected and (ui->modo->currentIndex()==1))
//    {
//        SendMessage();
//    }
//}
////Envia los PWM de las roscas
//void GUIPanel::on_Knob_Verde_valueChanged()
//{
//    if (connected and (ui->modo->currentIndex()==1))
//    {
//        SendMessage();
//    }
//}
////Envia los PWM de las roscas
//void GUIPanel::on_Knob_Azul_valueChanged()
//{
//    if (connected and (ui->modo->currentIndex()==1))
//    {
//        SendMessage();
//    }
//}



//Envia el comando indicando el modo de los botones
//void GUIPanel::on_async_toggled()
//{
//    if (connected)
//    {
//        SendMessage();
//    }
//}

void GUIPanel::on_tempslider_sliderReleased()
{
    if (connected and (ui->pushButton_temp->text()=="Apaga"))
    {
        SendMessage();
    }
}

void GUIPanel::on_bmeslider_sliderReleased()
{
    if (connected and (ui->pushButton_bme->text()=="Apaga"))
    {
        SendMessage();
    }
}

void GUIPanel::on_habilitar_alarma_toggled()
{
    if (connected)
    {
        SendMessage();
    }
}

void GUIPanel::on_umbral_alarma_sliderReleased()
{
    if (connected and (ui->habilitar_alarma->isChecked()))
    {
        SendMessage();
    }
}

void GUIPanel::on_umbral_alarma_bme_sliderReleased()
{
    if (connected and (ui->habilitar_alarma_bme->isChecked()))
    {
        SendMessage();
    }
}

void GUIPanel::on_habilitar_alarma_bme_toggled()
{
    if (connected)
    {
        SendMessage();
    }
}
