#include <iostream>
#include <iomanip>
#include <fstream>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <sstream>
#define DB "test.db"
using namespace std;

/* Global variables */
/* sqlite database pointer */
sqlite3 *dbfile;
sqlite3_stmt *stmt;
bool isOpen = false;
/**************************/

/* Function Prototypes */
bool ConnectDB();
bool CreateTables();
bool InsertArtist(string []);
bool InsertTrack(string []);
bool InsertCd(string []);
bool Update(string []);
bool Delete(string);
bool openFileIn(fstream &, string);
bool openFileOut (fstream &, string);
bool DisconnectDB();
void menu();
void displayMenuOptions();
void parseArtists(fstream &);
void parseCds(fstream &);
void parseTracks(fstream &); 
string select(string []);
static int callback(void *, int , char **, char **);
static int callbackFileOut(void *, int, char **, char **);
/***************************************************/

int main () {
	fstream file;
	string fileName[4] = {"artist.txt", "cd.txt", "track.txt"};
	
	cout << "Program started" << endl;
	
	// Connect to database
	if (ConnectDB()) {
		cout << "Connected to database successfully!" << endl << endl;
		
		// Create all tables that are not already present
		cout << "Attempting to create tables..." << endl;
		bool flag = false;
		if (CreateTables()) {
			cout << "Successfully created all tables!" << endl << endl;
			flag = true;
		}
		else {
			cout << "Tables already present, skipping table creation and automatic population" << endl << endl;
		}
		
		// If the tables did not exist prior to the program running, populate them now.
		// If the tables were already present, they do not need to be populated with default data.
		if (flag) {
			// Read each file and do the required operations on each one
			for (int i = 0; i < 3; i++) {
				// Start by reading the file
				if (openFileIn(file, fileName[i])) {
					cout << "File:" << fileName[i] << " opened successfully!" << endl;
					cout << "Reading data from file..." << endl;
					
					if (i == 0) {
						parseArtists(file);
					}
					else if (i == 1) {
						parseCds(file);
					}
					else {
						parseTracks(file);
					}
					file.close();
					cout << "Done Reading file: " << fileName[i] << endl << endl;
				}
				else {
					cout << "Could not open file: " << fileName[i] << endl << endl;
				}
			}
		}
		
		// Display the menu for user interaction
		menu();
		
		// Select data
		string table = "cd";
		string column = "*";
		/* string result =  select(table, column);*/
		//cout << endl << "Select result in main: " << endl << result << endl << endl;
	}
	else {
		cout << "Couldn't connect to database. " << endl << endl;
		exit(EXIT_FAILURE);
	}
	
	return 0;
}

/* Display the menu for the user to interact with.
Options are: 1 = Update a row in the database
			 2 = Exit the program
Program will continue execution until user enters 2.
Only  options are to Update and Exit */
void menu() {
	string data[6];
	string tableVal;
	string input;
	while (true) {	
		displayMenuOptions();
		cin.clear();
		getline(cin, input);
		
		switch(atoi(input.c_str())) {
			// Update a specific record on a table
			case 1:
				cout << "Enter table to update (cd, track, astist): ";
				getline(cin, data[0]);
				
				cout << endl << "Enter column to update: ";
				getline(cin, data[1]);
				
				cout << endl << "Enter the new value: ";
				getline(cin, data[2]);
				
				cout << endl << "Enter the column to filter on: ";
				getline(cin, data[3]);
				
				cout << endl << "Enter the value of the filter column: ";
				getline(cin, data[4]);
				
				if (Update(data)) {
					cout << "Updated Successfully!" << endl;
				}
				else {
					cout << "Error while updating." << endl;
				}
				break;
			
			// Select data from the relevant table
			case 2:
				cout << endl << "Enter column to select or * for all: ";
				getline(cin, data[0]);
				
				cout << endl << "Enter table to select from(cd, artist, track): ";
				getline (cin, data[1]);
				
				select(data);
				break;
			
			// Delete all data from the table
			case 3:
				cout << "Enter table to clear(cd, artist, track): ";
				getline(cin, tableVal);
				if(Delete(tableVal)) {
					cout << "Table cleared successfully!" << endl;
				}
				else {
					cout << "Error deleting." << endl;
				}
				break;
			
			// Exit the program
			case 4:
				cout << "Disconnecting from the database..." << endl;
				if (DisconnectDB()) {
					cout << "Successfully disconnected!" << endl;
				}
				else {
					cout << "Error occurred while disconnecting from database." << endl;
				}
				exit(EXIT_SUCCESS);
		}
		// Reset input variables
		input = "";
		for (int i = 0; i <6; i++) {
			data[i] = "";
		}
	}
}

/* Display all the available options to the user */
void displayMenuOptions() {
	cout << "1) Update Column\n2) Select\n3) Delete\n4) Exit" << endl;
}

/* Attempt to open the file that is passed.
Will return false if failure while opening,
returns true if opened successfully */
bool openFileIn (fstream &file, string filename) {
	file.open(filename.c_str(), ios::in);
	if (file.fail()) {
		return false;
	}
	return true;
}

bool openFileOut (fstream &file, string filename) {
	file.open(filename.c_str(), ios::app | ios::out);
	if (file.fail()) {
		return false;
	}
	return true;
}
/* Parse artists and their Id's from the artist file, then insert them 
 individually into the database. */
void parseArtists(fstream &file) {
	string line;
	string delim   = "|";
	int startPos   = 0;
	int endPos     = 0;
	int numSuccess = 0;
	int numError   = 0;
	string artistData[3];

	// Get the first line of the file
	getline(file, line);
	endPos = line.find(delim);
	if (endPos != string::npos) {
		while (file) {
			// Get the artist ID
			artistData[0] = line.substr(startPos, endPos);
			
			// Update the pos pointers to get the artist name
			startPos = endPos + 1;
			endPos = line.length() - 1;
			
			// Get artist name
			artistData[1] = line.substr(startPos, endPos);
			
			// Insert artists into the artist table individually
			if (InsertArtist(artistData)) {
				numSuccess++;
			}
			else {
				numError++;
			}
			
			// Get the next line and reset pos pointers
			getline(file, line);
			endPos = line.find(delim);
			startPos = 0;
		}
	}
	else {
		cout << "No delimiter was found on the artist file line!" << endl;
	}
	cout << "Successfully inserted " << numSuccess << " artists!" << endl;
	cout << "Errors occurred while inserting artists: " << numError << endl;
}

/* Parse cd's and their attributes from the cd file, then insert them 
individually into the database.*/
void parseCds(fstream &file) {
	string line;
	string delim   = "|";
	int startPos   = 0;
	int endPos     = 0;
	int numSuccess = 0;
	int numError   = 0;
	string cdData[4];

	// Get the first line of the file
	getline(file, line);
	endPos = line.find(delim);
	if (endPos != string::npos) {
		while (file) {
			// Go through each line multiple times to parse out the
			// required information
			for (int i = 0; i < 4; i++) {
				cdData[i] = line.substr(startPos, endPos);
				
				// Sanitize data when i == 1 || i == 2 due to multiple delimiters
				// being present in this section of data
				if (i == 1 || i == 2) {
					int tempStartPos = 0;
					int tempEndPos = 0;
					tempEndPos = cdData[i].find(delim, tempStartPos);
					cdData[i] = cdData[i].substr(tempStartPos, tempEndPos);
				}

				//cout /*<< "Start: " << startPos << endl << "End: " << endPos*/ << endl;
				if (endPos == string::npos) {
					endPos = line.length() - 1;
				}
				//cout << "CdData[" << i << "]: " << cdData[i] << endl;
				
				startPos = endPos + 1;
				endPos = line.find(delim, startPos);
			}
			// Insert the cd's into the cd table individually
			if (InsertCd(cdData)) {
				numSuccess++;
			}
			else {
				numError++;
			}
			
			// Get the next line and reset pos pointers
			getline(file, line);
			endPos = line.find(delim);
			startPos = 0;
		}
	}
	else {
		cout << "No delimiter was found on the cd file line!" << endl;
	}
	cout << "Successfully inserted " << numSuccess << " cd's!" << endl;
	cout << "Errors occurred while inserting cd's: " << numError << endl;
}

/* Parse cd's and their attributes from the cd file, then insert them 
individually into the database.*/
void parseTracks(fstream &file) {
	string line;
	string delim   = "|";
	int startPos   = 0;
	int endPos     = 0;
	int numSuccess = 0;
	int numError   = 0;
	string trackData[4];

	// Get the first line of the file
	getline(file, line);
	endPos = line.find(delim);
	if (endPos != string::npos) {
		while (file) {
			// Go through each line multiple times to parse out the
			// required information
			for (int i = 0; i < 3; i++) {
				trackData[i] = line.substr(startPos, endPos);
				
				// Sanitize data when i == 1 || i == 2 due to multiple delimiters
				// being present in this section of data
				if (i == 1 || i == 2) {
					int tempStartPos = 0;
					int tempEndPos = 0;
					tempEndPos = trackData[i].find(delim, tempStartPos);
					trackData[i] = trackData[i].substr(tempStartPos, tempEndPos);
				}

				//cout /*<< "Start: " << startPos << endl << "End: " << endPos*/ << endl;
				if (endPos == string::npos) {
					endPos = line.length() - 1;
				}
				//cout << "trackData[" << i << "]: " << trackData[i] << endl;
				
				startPos = endPos + 1;
				endPos = line.find(delim, startPos);
			}
			// Insert the track into the track table
			if (InsertTrack(trackData)) {
				numSuccess++;
			}
			else {
				numError++;
			}
			// Get the next line and reset pos pointers
			getline(file, line);
			endPos = line.find(delim);
			startPos = 0;
		}
	}
	else {
		cout << "No delimiter found on the track file line!" << endl; 
	}
	cout << "Successfully inserted " << numSuccess << " tracks!" << endl;
	cout << "Errors occurred while inserting tracks: " << numError << endl;
}

/* Select specific data or all data from a specified table */
string select (string data[]) {
	if (isOpen) {
		stringstream queryStrm;
 		//const char* data = "";
		char *zErrMsg = 0;

		queryStrm << "select " << data[0] << " from " << data[1] << ";";
		string sqlStr = queryStrm.str();
		cout <<  setw(20) << left << "Column";
		cout << "Value" << endl;
		for (int i = 0; i < 41; i++) {
			cout << "-";
		}
		cout << endl;
		
 		int rc = sqlite3_exec(dbfile, sqlStr.c_str(), callback, /*(void*)data*/0, &zErrMsg);
		
		// Write select to file
		string filename = "songDbOutput.txt";
		remove(filename.c_str());
		sqlite3_exec(dbfile, sqlStr.c_str(), callbackFileOut, /*(void*)data*/0, &zErrMsg);
		if( rc != SQLITE_OK ){
			fprintf(stderr, "SQL error: %s\n", zErrMsg);
			sqlite3_free(zErrMsg);
		}
		else{
			cout << "Select completed successful!" << endl;
		} 
	}
	else {
		return  "Error attempting select: not connected to a database.\n";
	}
}

/* Attempt to connect to the database. If successful,
returns true. Else, false is returned*/
bool ConnectDB () {
	if (sqlite3_open_v2(DB, &dbfile, SQLITE_OPEN_READWRITE, NULL ) == SQLITE_OK) {
		isOpen = true;
		return true;
	}
	return false;
}

/* If the connection to the database is open, it will
be closed.*/
bool DisconnectDB () {
	if (isOpen) {
		if (sqlite3_close(dbfile) == SQLITE_OK) {
			isOpen = false;
			return true;
		}
		else {
			return false;
		}
	}
	else {
		cout << "Cannot disconnect; program not connected to a database!" << endl;
	}
}

/* Create all the tables that are required;
artist, track, and cd tables with their respective
columns and attributes*/
bool CreateTables() {
	const int NUMTABLES = 3;
	int numSuccess    = 0;
	int numError      = 0;
	string queries[4] = {"create table cd (" \
		"id INTEGER AUTO_INCREMENT NOT NULL PRIMARY KEY," \
		"title VARCHAR(70) NOT NULL," \
		"artist_id INTEGER NOT NULL," \
		"catalogue VARCHAR(30) NOT NULL);", "create table artist (" \
		"id INTEGER AUTO_INCREMENT NOT NULL PRIMARY KEY,"\
		"name VARCHAR(100) NOT NULL);", "create table track ("\
		"cd_id INTEGER NOT NULL,"\
		"track_id INTEGER NOT NULL,"\
		"title VARCHAR(70),"\
		"PRIMARY KEY(cd_id, track_id));", "create table track ("\
		"cd_id INTEGER NOT NULL,"\
		"track_id INTEGER NOT NULL,"\
		"title VARCHAR(70),"\
		"PRIMARY KEY(cd_id, track_id));"
	};

	for (int i = 0; i < NUMTABLES; i++) {
		//cout << "Create query: " << queries[i] << endl;
		if (sqlite3_prepare(dbfile, queries[i].c_str(), -1, &stmt, 0) == SQLITE_OK) {
			sqlite3_step(stmt);
			sqlite3_finalize(stmt);
			numSuccess++;
		}
		else {
			cout << "Error occurred creating table." <<  endl;
			numError++;
			return false;
		}
	}
	cout << "Successfully created " << numSuccess << " tables!" << endl;
	return true;
}

/* Function to insert data into artist table */
bool InsertArtist (string artistData[]) {
	stringstream queryStrm;
	queryStrm << "insert into artist(id, name) values('" << artistData[0] << "', '"<< artistData[1] << "');";
	string sqlStr = queryStrm.str();
	char *sql = &sqlStr[0];
	
	if (sqlite3_prepare(dbfile, sql, -1, &stmt, 0) == SQLITE_OK) {
		sqlite3_step(stmt);
		sqlite3_finalize(stmt);
	}
	else {
		return false;
	}
	return true;
}

/* Function to insert data into cd table */
bool InsertCd (string cdData[]) {
	stringstream queryStrm;
	queryStrm << "insert into cd(id, title, artist_id, catalogue) values(" << cdData[0] << ", '" << cdData[1] << "', " << cdData[2] << ", '" << cdData[3] << "');";
	string sqlStr = queryStrm.str();
	char *sql = &sqlStr[0];
	
	if (sqlite3_prepare(dbfile, sql, -1, &stmt, 0) == SQLITE_OK) {
		sqlite3_step(stmt);
		sqlite3_finalize(stmt);
	}
	else {
		return false;
	}
	return true;
}

/* Function to insert data into track table */
bool InsertTrack (string trackData[]) {
	stringstream queryStrm;
	queryStrm << "insert into track(cd_id, track_id, title) values(" << trackData[0] << ", " << trackData[1] << ", '" << trackData[2] << "');";
	string sqlStr = queryStrm.str();
	char *sql = &sqlStr[0];
	
	if (sqlite3_prepare(dbfile, sql, -1, &stmt, 0) == SQLITE_OK) {
		sqlite3_step(stmt);
		sqlite3_finalize(stmt);
	}
	else {
		return false;
	}
	return true;
}

/* Update the specified table and column, based on the WHERE clause
that is built by the user. */
bool Update(string data[]) {
	stringstream queryStrm;
	queryStrm << "UPDATE " << data[0] << " SET " << data[1] << "='" << data[2] << "' WHERE " << data[3] << "='" << data[4] << "';";
	string sqlStr = queryStrm.str();
	char *sql = &sqlStr[0];
	
	if (sqlite3_prepare(dbfile, sql, -1, &stmt, 0) == SQLITE_OK) {
		sqlite3_step(stmt);
		sqlite3_finalize(stmt);
	}
	else {
		return false;
	}
	return true;
	
}

/* Update the specified table and column, based on the WHERE clause
that is built by the user. */
bool Delete(string table) {
	stringstream queryStrm;
	queryStrm << "DELETE FROM " << table << ";";
	string sqlStr = queryStrm.str();
	char *sql = &sqlStr[0];
	
	if (sqlite3_prepare(dbfile, sql, -1, &stmt, 0) == SQLITE_OK) {
		sqlite3_step(stmt);
		sqlite3_finalize(stmt);
	}
	else {
		return false;
	}
	return true;
	
}
static int callbackFileOut(void *data, int argc, char **argv, char **azColName){
	fstream fout;
	stringstream sstream;
	string filename = "songDbOutput.txt";
	string str;

	if(openFileOut(fout, filename)) {
		for(int i = 0; i < argc; i++){
			sstream << setw(20) << left << azColName[i] << argv[i]  ? argv[i] : "NULL\n";
			str = sstream.str();
			sstream.str(string());
			str += "\n";
			fout << str.c_str();
		}
		fout.close();
	}
	else {
		cout << "Error while opening file for output." << endl;
	}
	
	return 0;
}

static int callback(void *data, int argc, char **argv, char **azColName){
	for(int i = 0; i < argc; i++){
		printf("%-20s  %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
	}
	cout << endl;
	return 0;
}