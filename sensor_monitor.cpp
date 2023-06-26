#include <iostream>
#include <cstdlib>
#include <chrono>
#include <ctime>
#include <thread>
#include <unistd.h>
#include "json.hpp" // json handling
#include "mqtt/client.h" // paho mqtt
#include <iomanip>
#include <random>

#define QOS 1
#define BROKER_ADDRESS "tcp://localhost:1883"

using namespace std;

string generate_machine_id(){
    // Get the unique machine identifier, in this case, the hostname.
    char hostname[1024];
    gethostname(hostname, 1024);
    std::string machineId(hostname);
    return machineId;

}

// Função para gerar um UUID aleatório para o sensor
string generate_sensor_ids(string machine_id){
   
    string sensors[17] = {"Kok","cpu", "disco", "mem","abd","cghce","yhask","ytda","HSds","uydkd","hdkas","ada","hes""lij","peq","sdas","bhr"};
    string sensor_id ;

    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dis(0, 16);

    for (int i = 0; i < 16; ++i) {
        int random_index = dis(gen);
        sensor_id =  sensors[random_index];
    }

    return sensor_id;

}

int generate_values(string sensor_ids){
    random_device rd;
    mt19937 gen(rd());
    float value = 0.0;
    if(strncmp(sensor_ids.c_str(),"cpu",3)){
        uniform_int_distribution<> dis(10, 100);
        for (int i = 10; i < 99; ++i) {
            int random_index = dis(gen);
            value = random_index;
        }

    }else if(strncmp(sensor_ids.c_str(),"mem",3)){
        uniform_int_distribution<> dis(1, 12);
        for (int i = 1; i < 11; ++i) {
            int random_index = dis(gen);
            value = random_index;
        }

    }else if(strncmp(sensor_ids.c_str(),"disco",3)){
        uniform_int_distribution<> dis(2, 100);
        for (int i = 2; i < 99; ++i) {
            int random_index = dis(gen);
            value = random_index;
        }
    }else{
        uniform_int_distribution<> dis(100, 500);
        for (int i = 100; i < 500; ++i) {
            int random_index = dis(gen);
            value = random_index;
        }
    }
    return value;
}

int main(int argc, char* argv[]) {
    int values;

    std::string clientId = "sensor-monitor";
    mqtt::client client(BROKER_ADDRESS, clientId);

    // Connect to the MQTT broker.
    mqtt::connect_options connOpts;
    connOpts.set_keep_alive_interval(20);
    connOpts.set_clean_session(true);

    try {
        client.connect(connOpts);
    } catch (mqtt::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    std::clog << "connected to the broker" << std::endl;
    string machine_id = generate_machine_id();
    thread machine_thread([&client, &machine_id]() {
        cout<<"New machine connected... "<<machine_id<<endl;
        });


        while (true) {
       // Get the current time in ISO 8601 format.
        auto now = std::chrono::system_clock::now();
        std::time_t now_c = std::chrono::system_clock::to_time_t(now);
        std::tm* now_tm = std::localtime(&now_c);
        std::stringstream ss;
        ss << std::put_time(now_tm, "%FT%TZ");
        std::string timestamp = ss.str();
        values =  generate_values(generate_sensor_ids(machine_id));
        // Construct the JSON message.
        string sensor_id1 = generate_sensor_ids(generate_machine_id());
        nlohmann::json j;
        j["Sensor_id"] = sensor_id1;
        j["timestamp"] = timestamp;
        j["value"] = values;
        j["data-type"] = "int";


        // Publish the JSON message to the appropriate topic.
        std::string topic = "/sensors/" + machine_id + "/"+ sensor_id1;
        mqtt::message msg(topic, j.dump(), QOS, false);
        std::clog << "message published - topic: " << topic << " - message: " << j.dump() << std::endl;
        client.publish(msg);

        // Sleep for some time.
        std::this_thread::sleep_for(std::chrono::seconds(1));

            }


         machine_thread.join();


    return EXIT_SUCCESS;
}