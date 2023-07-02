const express = require('express');
const app = express();
const MongoClient = require('mongodb').MongoClient;

// MongoDB connection details
const url = 'mongodb://db:27017/?readPreference=primary&ssl=false&directConnection=true';
const dbName = 'sensors_data';
const collectionName = 'Alarms';

// Define a route to fetch data from MongoDB
app.get('/data', (req, res) => {
    MongoClient.connect(url, { useNewUrlParser: true, useUnifiedTopology: true }, (err, client) => {
        if (err) {
            console.error('Failed to connect to MongoDB:', err);
            res.status(500).send('Failed to connect to MongoDB');
            return;
        }

        const db = client.db(dbName);
        const collection = db.collection(collectionName);

        collection.find().toArray((err, data) => {
            if (err) {
                console.error('Failed to fetch data from MongoDB:', err);
                res.status(500).send('Failed to fetch data from MongoDB');
            } else {
                res.json(data);
            }
            client.close();
        });
    });
});

// Start the server
app.listen(3000, () => {
    console.log('Server listening on port 3000');
});
