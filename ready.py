#vc precisa installar python,
#faça, python3 pip install pymongo
#e dps python3 pip install flask
# faça uma atualização (sudo apt update)
#agora vc pode rodar o servidor fazendo 
#python3 ready.py.
#click ni pop up para abrir o site.

from pymongo import MongoClient
from flask import Flask, render_template, request

app = Flask(__name__)

# Connection details for MongoDB
uri = 'mongodb://db:27017'  # Replace with your MongoDB connection URI
databaseName = 'sensors_data'  # Replace with your database name
collectionName = input("Collection name (Alarms...cpu...mem): ") # Replace with your collection name

# Initialize MongoDB client
client = MongoClient(uri)
collection = client[databaseName][collectionName]


@app.route('/', methods=['GET', 'POST'])
def index():
    if request.method == 'POST':
        # Retrieve the number of records requested
        num_records = int(request.form.get('num_records'))

        # Retrieve the specified number of users from MongoDB
        users = list(collection.find().limit(num_records))
    else:
        # Default: Retrieve all users from MongoDB
        users = list(collection.find())

    # Render the template with the user data
    return render_template('index.html', users=users)

if __name__ == '__main__':
    app.run()
