#include "myClient.h"
#include "clientUtilityFunctions.h"

void* write_to_file(void* arg){
    try{
        int sock = *((int*) arg);
        DefaultIO *io = new SocketIO(sock);
        io->write("READY_TO_SAVE");
        // DefaultIO* io = ((DefaultIO*) arg);
        // io->write("");
        string path;
        getline(cin, path); // get path from user to save results
        io->saveData(path); // save data in the local path
        // io->write("");
        delete io;
    }catch(runtime_error& e){
        cout << e.what();
    }
    return 0;
}

// Method to send local data from client to server
void MyClient::sendData(string message, DefaultIO* dio) {
    DefaultIO *io = dio;
    
    cout << message << endl;
    string path;
    getline(cin, path);
    string fileContent;
    try {
        fileContent = io->readFile(path);
    } catch(...) {
        throw false;
    }

    io->write(fileContent);

    //  // Send the file in chunks
    // int bytesLeft = fileContent.length(); 
    // int BUFFER_SIZE = 4096;
   
    // // if fileContent is empty ///////////////////

    // while (bytesLeft > 0) { // while there are still bytes to send
    //     int bytesToSend = min(BUFFER_SIZE, bytesLeft);
    //     int result = io->writeFromFile(fileContent);
    //     // io->read(); // handle inner logics
    //     // if (result < 0) {
    //     //     throw false; // file is not valid
    //     // }
    //     bytesLeft -= result;
    // }
    // io->write("");

}

void MyClient::run(int argc, char** argv) {
    // make sure there are 3 arguments to activate client
    if (argc < 3) {
        cerr << "Usage: client.out IP_ADDRESS PORT" << endl;
        return;
    }

    // Parse the IP address and port number
    const char* ip_address = argv[1];
    const int port = stoi(argv[2]);

    // Create a socket
    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket < 0) {
        cerr << "Error creating socket" << endl;
        return;
    }

    // connect to the server
    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr(ip_address);
    serverAddr.sin_port = htons(port);
    if (inet_pton(AF_INET, ip_address, &serverAddr.sin_addr) <= 0) {
        cerr << "Error parsing IP address" << endl;
        return;
    }
   
    // Connect to the server
    if (connect(client_socket, (struct sockaddr *) &serverAddr, sizeof(serverAddr)) < 0) {
        cerr << "Error connecting to server" << endl;
        return;
    }

    pthread_t download_thread;
    DefaultIO *io = new SocketIO(client_socket);
    while(true){
        string input;
        string message = io->read();
        if(message == "Please upload your local train CSV file.") {
            try {
                sendData(message, io); // upload train data
                this_thread::sleep_for(chrono::milliseconds (100));
                // read another message from server
                // io->write(""); // handle inner logics
                string newMessage = io->read();   // iris_classified.csv
                sendData(newMessage, io); // upload test data
                this_thread::sleep_for(chrono::milliseconds (100));

            } catch(...) {
                 // path doesn't exist
                 io->write("");
            }        
            continue;    
        } 
        else if(message == "upload results:"){
            pthread_create(&download_thread, NULL, write_to_file, (void*) &client_socket); // can change to send io instead socket????
            pthread_detach(download_thread);
            this_thread::sleep_for(chrono::milliseconds (100));
            continue;
        }
        else if(message == "EXIT"){ // user pressed 8 so exit
            break;
        
        } else { 
            cout << message << endl;
            getline(cin, input);
            io->write(input);
        }
    }
//    pthread_t receive_thread; // create a receive thread
//    pthread_t send_thread; // create a send thread
//    // create the thread data:
//    ThreadData data;
//    data.io = new SocketIO(client_socket);
//    data.socket = client_socket;
//    pthread_mutex_init(&data.mutex, NULL);
//    pthread_cond_init(&data.cond, NULL);
//    while (!data.messages.empty()) {
//        data.messages.pop();
//    }
    // send threads to interact with server
//    pthread_create(&receive_thread, NULL, receive_from_server, &data);
//    pthread_create(&send_thread, NULL, send_to_server, &data);

    // wait for the receive thread to finish
//    pthread_join(receive_thread, NULL);
//    pthread_join(send_thread, NULL);
    //pthread_mutex_destroy();

    delete io;

    // close the socket
    close(client_socket);

    // Close client
    return;
}

 

