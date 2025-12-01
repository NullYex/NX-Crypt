/**
 * PROJECT: Secure File Encryptor/Decryptor
 * AUTHOR: By NullYex Team - B.tech Ai&Ds (IBM) - Amrit Ranjan, Kamran Alvi, Farhan Khalid
 * DESCRIPTION: A C++ CLI tool to encrypt files using XOR logic and Salted Hashing.
 */

#include <iostream> //For printing to the screen (cout, cin)
#include <fstream>  //For reading and writing files
#include <string>   //For using text strings
#include <vector>   //For using dynamic arrays (buffers)
#include <cstdlib>  //For standard tools like random numbers
#include <ctime>    //For getting the current time (to seed random numbers)
using namespace std;

//CONFIGURATION
//1MB Buffer: We read 1 Megabyte at a time.
//Faster than reading byte-by-byte because it reduces hard drive access.
const size_t BUFFER_SIZE = 1024 * 1024; 
//The file extension we append to encrypted files
const string EXTENSION = ".NullYex";
//A "Stamp" we put at the start of the file so we know we created it.
const string BRANDING = "By_NullYex"; 

//FUNCTIONS DECLARATIONS
//Generates random text (Salt) to mix with the password.
//This prevents hackers from guessing passwords using pre-calculated tables.
string generateSalt(int length) {
    string salt = "";
    const char charset[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    for (int i = 0; i < length; ++i) {
        //Pick a random character from the charset
        salt += charset[rand() % (sizeof(charset) - 1)];
    }
    return salt;
}

//Turns a string (Password + Salt) into a unique number (Hash).
//This is a "One-Way" function. You can't turn the number back into the password.
unsigned long long simpleHash(const string& data) {
    unsigned long long hash = 5381; //Starting number (magic constant)
    for (char c : data) {
        //Mix the bits: (hash * 33) + character
        hash = ((hash<<5) + hash) + c; 
    }
    return hash;
}

//Checks if the file starts with our "By_NullYex" branding tag.
bool isEncryptedByNullyex(const string& path) {
    ifstream file(path, ios::binary);
    if (!file.is_open()) return false;

    vector<char> buffer(BRANDING.size());
    
    //Read the first few bytes
    if (file.read(buffer.data(), BRANDING.size())) {
        string fileHeader(buffer.begin(), buffer.end());
        return fileHeader == BRANDING; //True if they match
    }
    return false;
}

//Removes quotes "" from file paths (common issue on Windows)
string cleanPath(string path) {
    if (path.length() >= 2 && path.front() == '"' && path.back() == '"') {
        return path.substr(1, path.length() - 2);
    }
    return path;
}

//Gets the extension (e.g., ".png") from a filename
string getFileExtension(const string& path) {
    size_t dotPos = path.find_last_of('.');
    if (dotPos != string::npos) return path.substr(dotPos);
    return "";
}

//Removes the extension from a filename
string removeExtension(const string& path) {
    size_t dotPos = path.find_last_of('.');
    if (dotPos != string::npos) return path.substr(0, dotPos);
    return path;
}

//THE CORE LOGIC: PROCESS FILE
bool processFile(const string& inputFileName, string password, bool encryptMode) {
    //Step 1: Open the Input File
    //ios::binary is CRITICAL. It tells C++ "Read this exact data."
    //Without it, C++ might change newlines, corrupting images or videos.
    ifstream inputFile(inputFileName, ios::binary);
    if (!inputFile.is_open()) {
        cout<<"\n[DEBUG ERROR] File check failed!"<<endl;
        return false;
    }

    string outputFileName;
    string originalExtension;
    unsigned long long keyIndex = 0; //Tracks our position for the "Rolling Key"
    size_t passwordLength;

    //IF WE ARE ENCRYPTING...
    if (encryptMode) {
        if (password.empty()) {
            cout<<"Error: Password cannot be empty!"<<endl;
            return false;
        }
        passwordLength = password.length();

        //Prepare Output Filename (e.g., "image.png" -> "image.NullYex")
        originalExtension = getFileExtension(inputFileName);
        string basePath = removeExtension(inputFileName);
        outputFileName = basePath + EXTENSION;
        cout<<"Target output: "<<outputFileName<<endl;

        ofstream outputFile(outputFileName, ios::binary);
        if (!outputFile.is_open()) {
            cout<<"Error: Could not create output file."<<endl;
            return false;
        }

        //WRITE THE HEADER (The Envelope)
        //1. Branding: "By_NullYex"
        outputFile.write(BRANDING.c_str(), BRANDING.size());

        //2. Salt: Generate 8 random chars and save them
        string salt = generateSalt(8);
        outputFile.write(salt.c_str(), salt.length());

        //3. Hash: Calculate secure hash and save it
        //(This allows us to verify the password later without storing the actual password)
        unsigned long long passHash = simpleHash(password + salt);
        
        //'reinterpret_cast' treats the number as raw bytes so we can write it to the file
        outputFile.write(reinterpret_cast<const char*>(&passHash), sizeof(passHash));

        //4. Extension: Save the original extension (encrypted)
        unsigned char extLen = (unsigned char)originalExtension.length();
        
        //Encrypt the length byte
        char keyChar = password[keyIndex % passwordLength];
        char encryptedLen = extLen ^ (keyChar + (char)keyIndex);
        outputFile.put(encryptedLen);
        keyIndex++;

        //Encrypt the extension letters (e.g., 'p', 'n', 'g')
        for (char c : originalExtension) {
            char k = password[keyIndex % passwordLength];
            outputFile.put(c ^ (k + (char)keyIndex));
            keyIndex++;
        }
        
        //PROCESS THE FILE BODY
        vector<char> buffer(BUFFER_SIZE); //1MB Buffer
        cout<<"Encrypting Please wait..."<<endl;

        size_t pIndex = keyIndex % passwordLength; //Optimization variable

        while (inputFile.read(buffer.data(), BUFFER_SIZE) || inputFile.gcount() > 0) {
            streamsize bytesRead = inputFile.gcount(); //How many bytes did we actually read?

            //Inject Branding Tag periodically (every 1MB)
            outputFile.write(BRANDING.c_str(), BRANDING.size());

            //The Encryption Loop
            for (streamsize i = 0; i < bytesRead; ++i) {
                //1. Get current password character
                char baseKey = password[pIndex];
                
                //Move to next password character (loop back if at end)
                pIndex++;
                if (pIndex == passwordLength) pIndex = 0;

                //2. Create Rolling Key: (PasswordChar + CurrentPosition)
                //This ensures the key changes for every single byte!
                char rollingKey = baseKey + (char)keyIndex;
                
                //3. Perform XOR Operation (The Magic)
                buffer[i] = buffer[i] ^ rollingKey;
                keyIndex++;
            }
            //Write encrypted chunk to file
            outputFile.write(buffer.data(), bytesRead);
            if (inputFile.eof()) break;
        }
        outputFile.close();

    //IF WE ARE DECRYPTING...
    } else {
        //1. Verify Branding (Is this actually our file?)
        vector<char> sigBuffer(BRANDING.size());
        if (!inputFile.read(sigBuffer.data(), BRANDING.size())) {
            cout<<"Error: File too short."<<endl;
            return false;
        }
        
        string fileSig(sigBuffer.begin(), sigBuffer.end());
        if (fileSig != BRANDING) {
            cout<<"Error: Invalid file format! Missing branding."<<endl;
            return false;
        }

        //Save position. We will come back here if the password is wrong.
        streampos authStartPos = inputFile.tellg();

        //2. Password Check Loop
        while (true) {
            if (password.empty()) {
                cout<<"Error: Password cannot be empty."<<endl;
            } else {
                //Reset file reader to the start of the security section
                inputFile.clear();
                inputFile.seekg(authStartPos);
                keyIndex = 0;
                passwordLength = password.length();

                //A. Read the Salt from the file
                vector<char> saltBuffer(8);
                if (!inputFile.read(saltBuffer.data(), 8)) {
                    cout<<"Error: File corrupted."<<endl;
                    return false;
                }
                string salt(saltBuffer.begin(), saltBuffer.end());

                //B. Read the correct Hash from the file
                unsigned long long storedHash;
                if (!inputFile.read(reinterpret_cast<char*>(&storedHash), sizeof(storedHash))) {
                    cout<<"Error: File corrupted."<<endl;
                    return false;
                }

                //C. Calculate Hash of the user's input (Input + Salt)
                unsigned long long inputHash = simpleHash(password + salt);

                //D. Compare! Does Input Hash == Stored Hash?
                if (inputHash == storedHash) {
                    cout<<"Password verified. Access granted."<<endl;
                    break; //Success! Exit the retry loop.
                } else {
                    cout<<"\n[ACCESS DENIED] Incorrect Password!"<<endl;
                }
            }

            //Retry Logic
            char retryChoice;
            cout<<"Would you like to try again? (y/n): ";
            cin>>retryChoice;
            cin.ignore(10000, '\n'); //Clear 'enter' key from buffer

            if (retryChoice == 'y' || retryChoice == 'Y') {
                cout<<"Enter password again: ";
                getline(cin, password);
                cout<<endl;
            } else {
                cout<<"Decryption aborted by user."<<endl;
                return false; 
            }
        }

        //3. Read Hidden Extension
        char encryptedLen;
        inputFile.get(encryptedLen); //Read length byte

        //Decrypt length
        char keyChar = password[keyIndex % passwordLength];
        unsigned char extLen = encryptedLen ^ (keyChar + (char)keyIndex);
        keyIndex++;

        //Decrypt extension characters
        string restoredExtension = "";
        for (int i = 0; i < extLen; i++) {
            char c;
            inputFile.get(c);
            char k = password[keyIndex % passwordLength];
            restoredExtension += (c ^ (k + (char)keyIndex));
            keyIndex++;
        }

        cout<<"Restored extension: "<<restoredExtension<<endl;
        string basePath = removeExtension(inputFileName);
        outputFileName = basePath + restoredExtension;
        
        ofstream outputFile(outputFileName, ios::binary);
        if (!outputFile.is_open()) {
            cout<<"Error: Could not create output file."<<endl;
            return false;
        }

        //4. Decrypt Body
        vector<char> buffer(BUFFER_SIZE);
        vector<char> brandSkip(BRANDING.size());
        cout<<"Decrypting Please wait..."<<endl;

        size_t pIndex = keyIndex % passwordLength;

        while (true) {
            //SKIP Branding Tag (It's not part of the original file)
            if (!inputFile.read(brandSkip.data(), BRANDING.size())) break;

            //Read Encrypted Chunk
            if (inputFile.read(buffer.data(), BUFFER_SIZE) || inputFile.gcount() > 0) {
                streamsize bytesRead = inputFile.gcount();

                //Decrypt Logic (Same as encryption!)
                for (streamsize i = 0; i < bytesRead; ++i) {
                    char baseKey = password[pIndex];
                    pIndex++;
                    if (pIndex == passwordLength) pIndex = 0;

                    char rollingKey = baseKey + (char)keyIndex;
                    buffer[i] = buffer[i] ^ rollingKey; //XOR reverses the change
                    keyIndex++;
                }
                outputFile.write(buffer.data(), bytesRead);
            }
            if (inputFile.eof()) break;
        }
        outputFile.close();
    }
    inputFile.close();
    return true;
}

//MAIN PROGRAM
int main() {
    //Seed the randomizer with the current time so 'random' numbers are different every run
    srand(time(0));
    string inputPath, password;
  
    cout<<" ============================================="<<endl;
    cout<<"|    Smart Secure File Encryptor/Decryptor    |"<<endl;
    cout<<" ---------------------------------------------"<<endl;
    cout<<"|     By NullYex Team - B.tech Ai&Ds (IBM)    |"<<endl;
    cout<<"|   Amrit Ranjan, Kamran Alvi, Farhan Khalid  |"<<endl;
    cout<<" ============================================="<<endl;

    //Infinite loop: keeps the program running until user types 'exit'
    while (true) {
        cout<<"\nEnter file path (copy as path) or (type 'e' / 'exit' to quit): ";
        getline(cin, inputPath);

        //Check if user wants to quit
        if (inputPath == "exit" || inputPath == "e" || inputPath == "EXIT") {
            cout<<"\nExiting program... by Nullyex, Goodbye!"<<endl;
            break;
        }

        //Fix path formatting (removes quotes if you copied as path)
        inputPath = cleanPath(inputPath);

        //Check if the file actually exists
        ifstream checkFile(inputPath);
        if (!checkFile.good()) {
            cout<<"Error: File not found at path: "<<inputPath<<endl;
            continue; //Restart the loop
        }
        checkFile.close();

        //SMART DETECTION
        //We peek at the file to see if it has our "By_NullYex" stamp.
        bool isAlreadyEncrypted = isEncryptedByNullyex(inputPath);
        bool encryptMode; 

        if (isAlreadyEncrypted) {
            cout<<"\nDetected: File is encrypted by Nullyex."<<endl;
            cout<<"Mode: DECRYPTION!"<<endl;
            encryptMode = false;
        } else {
            cout<<"\nDetected: Standard file (Not encrypted)."<<endl;
            cout<<"Mode: ENCRYPTION!"<<endl;
            encryptMode = true;
        }

        cout<<"\nEnter password: ";
        getline(cin, password);
        cout<<endl;

        //Run the main logic. It returns true if successful.
        if (processFile(inputPath, password, encryptMode)) {
            cout<<"\nSuccess! Operation Completed."<<endl;
        } else {
            cout<<"\nFailed! Please check the errors above or report to Nullyex."<<endl;
        }
    }
    return 0;
}
