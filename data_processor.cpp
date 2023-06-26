///Data_processor

#include <iostream>
#include <cstdlib>
#include <chrono>
#include <thread>
#include <unistd.h>
#include <bson.h>
#include <mongoc.h>
#include "json.hpp" 
#include "mqtt/client.h"
#include <vector>
#include <ctime>
#include <unordered_map>
#include <chrono>

#define QOS 1
#define BROKER_ADDRESS "tcp://localhost:1883"

using namespace std;
using json = nlohmann::json;

struct SensorData {
    string sensor__id;
    time_t timestamp;
};

std::vector<SensorData> sensorvec;

void insert_document(mongoc_collection_t *collection, std::string machine_id, std::string timestamp_str, int value) {
    bson_error_t error;
    bson_oid_t oid;
    bson_t *doc;
    
    std::tm tm{};
    std::istringstream ss{timestamp_str};
    ss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%SZ");
    std::time_t time_t_timestamp = std::mktime(&tm);

    doc = bson_new();
    BSON_APPEND_UTF8(doc, "machine_id", machine_id.c_str());
    BSON_APPEND_TIME_T(doc, "timestamp", time_t_timestamp);
    BSON_APPEND_INT32(doc, "value", value);

    if (!mongoc_collection_insert_one(collection, doc, NULL, NULL, &error)) {
        std::cerr << "Failed to insert doc: " << error.message << std::endl;
    }

    bson_destroy(doc);
}

std::vector<std::string> split(const std::string &str, char delim) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(str);
    while (std::getline(tokenStream, token, delim)) {
        tokens.push_back(token);
    }
    return tokens;
}


int main(int argc, char* argv[]) {
    std::string clientId = "clientId";
    mqtt::client client(BROKER_ADDRESS, clientId);

    // Initialize MongoDB client and connect to the database.
    mongoc_init();
    mongoc_client_t *mongodb_client = mongoc_client_new("mongodb://localhost:27017");
    mongoc_database_t *database = mongoc_client_get_database(mongodb_client, "sensors_data"); // replace with your database name

    // Create an MQTT callback.
    class callback : public virtual mqtt::callback {
        mongoc_database_t *db;
        std::unordered_map<std::string, std::chrono::steady_clock::time_point>  processlasttimestamp;

    public:
        callback(mongoc_database_t *db) : db(db) {}

        void message_arrived(mqtt::const_message_ptr msg) override {
        std::tm tm{};
        std::time_t time_t_stamp;
        
        double value;
        string timestamp;
        auto j = nlohmann::json::parse(msg->get_payload());

        std::string topic = msg->get_topic();
        auto topic_parts = split(topic, '/');

        std::string machine_id = topic_parts[2];
        std::string sensor_id = topic_parts[3];
        value = j["value"];

        // Check if the "timestamp" field is null
        if (!j["timestamp"].is_null()) {
            timestamp = j["timestamp"];
            // Perform further processing using the timestamp value
            std::istringstream ss{timestamp};
            ss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%SZ");
            time_t_stamp = std::mktime(&tm);

        }else{
             timestamp = "Null";
        }

        cout << "Machine id: " << machine_id << " Sensor id: " << sensor_id << " Value: " << value << endl;
        SensorData cpu;
       if (strncmp(sensor_id.c_str(), "cpu", 3) == 0 || strncmp(sensor_id.c_str(), "mem", 3) == 0 || strncmp(sensor_id.c_str(), "disco", 3) == 0) {
           
            cpu.sensor__id = sensor_id;
            cpu.timestamp =  time_t_stamp;
            auto currentTime = std::chrono::steady_clock::now();
            auto lastTimestamp = processlasttimestamp[cpu.sensor__id];
            std::chrono::duration<double> elapsedSeconds = currentTime - lastTimestamp;

            // Extract the elapsed time in seconds as a double value
            double elapsedSecondsValue = elapsedSeconds.count();
            int lastTwoDigits = static_cast<int>(elapsedSecondsValue)%100;
            cout << "Time in seconds: " <<  lastTwoDigits << endl;

            if (lastTwoDigits > 10) {
                string alarme =  "Sensor inativo por dez períodos de tempo previstos";
                cout << "Inserting data to db............." << endl;
                std::this_thread::sleep_for(std::chrono::milliseconds(2));
                // Get collection and persist the document.
                mongoc_collection_t *collection = mongoc_database_get_collection(db, "Alarms");
                insert_document(collection, machine_id, timestamp,value);
                mongoc_collection_destroy(collection);

            } else {

                if(strncmp(sensor_id.c_str(), "mem", 3) == 0 ){
                    if(value>17){
                        string mem_err = "Memoria está com 87% utilizado";
                         std::clog <<  mem_err << std::endl;
                        // Get collection and persist the document.
                        mongoc_collection_t *collection = mongoc_database_get_collection(db, sensor_id.c_str());
                        insert_document(collection, machine_id, timestamp,value);
                        mongoc_collection_destroy(collection);

                    }else {
                        // Get collection and persist the document.
                        string alarme = "Ok";
                        mongoc_collection_t *collection = mongoc_database_get_collection(db, sensor_id.c_str());
                        insert_document(collection, machine_id, timestamp,value);
                        mongoc_collection_destroy(collection);
                    }

                    }else if( strncmp(sensor_id.c_str(), "disco", 3) == 0){
                         if(value>90){
                            string mem_err = "disco está com 90% utilizado";
                            std::clog <<  mem_err << std::endl;
                            // Get collection and persist the document.
                            mongoc_collection_t *collection = mongoc_database_get_collection(db, sensor_id.c_str());
                            insert_document(collection, machine_id, timestamp,value);
                            mongoc_collection_destroy(collection);

                        }else {
                            // Get collection and persist the document.
                            string alarme = "Ok";
                            mongoc_collection_t *collection = mongoc_database_get_collection(db, sensor_id.c_str());
                            insert_document(collection, machine_id, timestamp,value);
                            mongoc_collection_destroy(collection);
                        }
                    }else if(strncmp(sensor_id.c_str(), "cpu", 3) == 0){
                        if(value>90){
                            string mem_err = "cpu está com 90% utilizado";
                            std::clog <<  mem_err << std::endl;
                            // Get collection and persist the document.
                            mongoc_collection_t *collection = mongoc_database_get_collection(db, sensor_id.c_str());
                            insert_document(collection, machine_id, timestamp,value);
                            mongoc_collection_destroy(collection);

                        }else {
                            // Get collection and persist the document.
                            string alarme = "Ok";
                            mongoc_collection_t *collection = mongoc_database_get_collection(db, sensor_id.c_str());
                            insert_document(collection, machine_id, timestamp,value);
                            mongoc_collection_destroy(collection);
                        }

                    }

               
            }
            // Update the lastTimestamp to restart the timer
            processlasttimestamp[cpu.sensor__id] = currentTime;
        }
    }
    };

    callback cb(database);
    client.set_callback(cb);

    // Connect to the MQTT broker.
    mqtt::connect_options connOpts;
    connOpts.set_keep_alive_interval(20);
    connOpts.set_clean_session(true);

    try {
        client.connect(connOpts);
        client.subscribe("/sensors/#", QOS);
    } catch (mqtt::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    // Cleanup MongoDB resources
    mongoc_database_destroy(database);
    mongoc_client_destroy(mongodb_client);
    mongoc_cleanup();

    return EXIT_SUCCESS;
}