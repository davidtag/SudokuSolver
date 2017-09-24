#include <iostream>
#include <fstream>
#include <string>
#include <array>
#include <random>
#include <time.h> /* clock_t, clock, CLOCKS_PER_SEC */
using namespace std;
const int MaxGuessRestarts = 50; //max restarts for guessing method (random algo)


/*****************************************************************************************************/
//01 CellAddress: definition, operators, functions, constants, and related data types
struct CellAddress{
	int rowNum;
	int colNum;
};
bool operator==(const CellAddress & a, const CellAddress & b) { //to check for structure equality
	if (a.rowNum == b.rowNum && a.colNum == b.colNum)
		return true;
	else
		return false;
}
bool operator!=(const CellAddress & a, const CellAddress & b) { //to check for structure inequality
	return !(a == b);
}
void PrintAddress(CellAddress Address) {
	cout << "(" << Address.rowNum << "," << Address.colNum << ")";  //function used for development/debugging
}
const CellAddress AddFlag1 = { -1,-1 }, AddFlag2 = { -2,-2 }; //special CellAddress constants used for indicating some outcome
const CellAddress Quadrants[9][9] = { 
									  {{0,0},{0,1},{0,2},{1,0},{1,1},{1,2},{2,0},{2,1},{2,2}},
									  {{0,3},{0,4},{0,5},{1,3},{1,4},{1,5},{2,3},{2,4},{2,5}},
									  {{0,6},{0,7},{0,8},{1,6},{1,7},{1,8},{2,6},{2,7},{2,8}},
									  {{3,0},{3,1},{3,2},{4,0},{4,1},{4,2},{5,0},{5,1},{5,2}},
									  {{3,3},{3,4},{3,5},{4,3},{4,4},{4,5},{5,3},{5,4},{5,5}},
									  {{3,6},{3,7},{3,8},{4,6},{4,7},{4,8},{5,6},{5,7},{5,8}},
									  {{6,0},{6,1},{6,2},{7,0},{7,1},{7,2},{8,0},{8,1},{8,2}},
									  {{6,3},{6,4},{6,5},{7,3},{7,4},{7,5},{8,3},{8,4},{8,5}},
									  {{6,6},{6,7},{6,8},{7,6},{7,7},{7,8},{8,6},{8,7},{8,8}},
									}; //addresses that make up the 9 quadrants (3x3 boxes) of a Sudoku grid
typedef array<CellAddress,20> CellSearchRange; //used in Cell class declaration
struct CellPossibleAddresses { //data type to hold possible addresses for a number to be stored in a row/col/quadrant
	array<CellAddress, 9> addresses;
	int numAddresses; //the number of non-trivial addressess stored
};
/*****************************************************************************************************/


/*****************************************************************************************************/
//02 Cell Class, Grid declaration, and related Grid functions
class Cell {
public:
	Cell() {
		value = 0; 
		valType = 0;
		quadrant = 0;
		address = { 0,0 }; 
		search_range = { {0,0} };
	}
	int GetValue() {
		return value;
	}
	char GetValueChar() {
		return value == 0 ? '.' : char(48 + value);
	}
	int GetValType() {
		return valType;
	}
	int GetQuadrant() {
		return quadrant;
	}
	void SetValue(int inputVal) {
		value = inputVal;
	}
	void SetValType(int inputValType) {
		valType = inputValType;
	}
	void SetQuadrant(int inputQuadrant) {
		quadrant = inputQuadrant;
	}
	CellAddress address; //address relative to Grid[][] array (defined below)
	CellSearchRange search_range; //Cells in the same row, colum, and quadrant as this Cell
private:
	int value;
	int valType; //0: Initialized Value; 1: Starting Val; 2: Solved val (deterministic); 3: a guess or solved after a guess was made
	int quadrant;
};
static Cell Grid[9][9]; //main object of program. Models Sudoku grid
Cell * GridPt(CellAddress a) {
	return &(Grid[a.rowNum][a.colNum]);
}
Cell * GridPt(int rowNum, int colNum) {
	return &(Grid[rowNum][colNum]);
}
typedef array<Cell *, 9> GridRange;
GridRange GridRangePt(char Class, int Index) {
//Returns an array of the 9 cells in a given Row(R), Column(C), or Quadrant(Q)
//Index is the number of the row, column, or quadrant
	GridRange temp;
	switch (Class) {
	case 'R':
		for (int i = 0; i < 9; i++) {
			temp.at(i) = GridPt(Index,i);
		} break;
	case 'C':
		for (int i = 0; i < 9; i++) {
			temp.at(i) = GridPt(i,Index);
		} break;
	case 'Q':
		for (int i = 0; i < 9;i++) {
			temp.at(i) = GridPt(Quadrants[Index][i]);
		} break;
	}
	return temp;
}
void InitializeGrid() {
//Initialize Grid[9][9] array. Executed once at run-time
	Cell *c;
	for (int row = 0; row < 9; row++) {
		for (int col = 0; col < 9; col++) {
			c = GridPt(row,col);

			//1) Set address
			//--------------------------------------------------------------
			(*c).address = { row, col };

			//2) Determine and set quadrant
			//--------------------------------------------------------------
			int tempQuadrant = -1;
			for (int quadrant = 0; quadrant < 9; quadrant++) {
				for (int memberCell = 0; memberCell < 9; memberCell++) {
					if ((*c).address == Quadrants[quadrant][memberCell]) {
						tempQuadrant = quadrant;
						break;
					}
				}
				if (tempQuadrant != -1) break;
			}
			(*c).SetQuadrant(tempQuadrant);

			//3) Determine and set search_range
			//--------------------------------------------------------------
			CellSearchRange tempSR = { { 0,0 } };
			int position = 0;
			//Go through all cell in col
			for (int i = 0; i < 9; i++) {
				if (i != (*c).address.rowNum) {
					tempSR[position] = { i, col };
					position++;
				}
			}
			//Go through all cells in row
			for (int j = 0; j < 9; j++) {
				if (j != (*c).address.colNum) {
					tempSR[position] = { row, j };
					position++;
				}
			}
			//Go through all cells in quadrant
			CellAddress tempAddress; bool InTempSR;
			for (int k = 0; k < 9; k++) {
				tempAddress = Quadrants[(*c).GetQuadrant()][k];
				if (tempAddress != (*c).address) {
					InTempSR = false; //check if already in the temp search range
					for (int i = 0; i < position; i++) {
						if (tempAddress == tempSR[i]) {
							InTempSR = true;
							break;
						}
					}
					if (!InTempSR) {
						tempSR[position] = tempAddress;
						position++;
					}
				}
			}
			(*c).search_range = tempSR;
		}
	}
}
int NumberOfEmptyCells() {
//Returns number of empty Cells in the grid (i.e. Value = 0)
	int count = 0;
	for (int row = 0;row < 9;row++) {
		for (int col = 0; col < 9; col++) {
			if (Grid[row][col].GetValue() == 0)
				count++;
		}
	}
	return count;
}
void ClearGuesses() {
	for (int row = 0;row < 9;row++) {
		for (int col = 0; col < 9; col++) {
			if (Grid[row][col].GetValType() == 3) { //ValType = 3 is all solutions after a guess was made, including the guess itself
				Grid[row][col].SetValue(0);
				Grid[row][col].SetValType(0);
			}
		}
	}
}
/*****************************************************************************************************/


/*****************************************************************************************************/
//03 CellPossibleValues: definition and functions
struct CellPossibleValues { //data type to hold possible values that can go in a Cell
	array<int, 9> values;
	int numValues; //the number of non-trivial values
};
bool IsPossibleVal(int Number, CellPossibleValues structPV) {
	//Basically checks if Number is in the values array of structPV
	bool temp = false;
	for (int i = 0; i < structPV.numValues; i++) {
		if (structPV.values.at(i) == Number) {
			temp = true;
			break;
		}
	}
	return temp;
}
/*****************************************************************************************************/


/*****************************************************************************************************/
//04 Sudoku Algorithms: Supporting Functions
CellPossibleValues PossibleValues(CellAddress add) {
//Returns an array of values (actually, a declared struct type) that can legally by in a cell (based on rules of Sudoku)
	CellPossibleValues temp;
	temp.values = { 0 };
	temp.numValues = 0;

	//Check which values (1 through 9) appear in search range
	bool ValUsed[9] = { 0 };//an indicator array. indicate if each of numbers 1 through 9 appear in search range
	CellAddress checkCell;
	int checkCellVal;
	for (int i = 0; i < 20; i++) {
		checkCell = GridPt(add)->search_range[i];
		checkCellVal = GridPt(checkCell)->GetValue();
		//cout << "(" << checkCell.rowNum << "," << checkCell.colNum << ") : " << checkCellVal << endl; //for debugging
		if (checkCellVal != 0)
			ValUsed[checkCellVal - 1] = 1;
	}

	//Update temp structure
	for (int N = 1; N <= 9; N++) {
		if (ValUsed[N - 1] == 0) {
			temp.values[temp.numValues] = N;
			temp.numValues++;
		}
	}

	return temp;
}
CellPossibleValues PossibleValues(const Cell & c) {
	return PossibleValues(c.address);
}
CellAddress MinPossibleValueAddress() {
//Return the address with the smallest number of possible values
//Error Values:
//		AddFlag1 ==> Puzzle is solved
//		AddFlag2 ==> Inconsistency
	CellAddress temp = AddFlag1;
	int currentMin = 10;
	int n;
	for (int row = 0;row < 9;row++) {
		for (int col = 0; col < 9; col++) {
			if (Grid[row][col].GetValue() == 0) { //i.e. cell is empty
				n = PossibleValues({ row,col }).numValues;
				if (n == 0) {
					//cell is empty but has no possible values => inconsistency
					temp = AddFlag2; //NOTE: this error is caught in the SolvePuzzle function
					goto ReturnValue;
				}
				else if (n < currentMin) {
					currentMin = n;
					temp = { row, col };
				}
			}
		}
	}
ReturnValue:
	return temp;
}
CellPossibleAddresses PossibleCells(int Number, char Class, int Index) {
// Finds all cells in the 'Class' (Row, Column, or Quadrant) that can contain the 'Number'
// and returns these cell addresses as a CellPossibleAddresses type
// The 'Index' of the 'Class' is the index of the class to look for
// E.g. PossibleCells(1, 'R', 6) returns a CellPossibleAddresses type
//		of all cells in row 6 that can contain the number 1
// Returns 0 CellPossibleAddresses type if no possible cells can hold the Number

//Number: The number to search for
//Class: 'R' for row; 'C' for column; 'Q' for quadrant
//Index: The number of Class. That is: Row 1, or Quadrant 5, etc. 
	CellPossibleAddresses temp;
	temp.addresses = { { 0,0 } };
	temp.numAddresses = 0;

	GridRange searchRange = GridRangePt(Class, Index);

	Cell * c;
	for (int i = 0; i < 9; i++) {
		c = searchRange.at(i);
		if (c->GetValue() == 0) { //only look at empty cells
			if (IsPossibleVal(Number, PossibleValues(*c))) {
				temp.addresses.at(temp.numAddresses) = c->address;
				temp.numAddresses++;
			}
		}
	}

	return temp;
}
CellPossibleValues MissingValues(char Class, int Index) {
	CellPossibleValues temp;
	temp.values = { 0 };
	temp.numValues = 0;

	GridRange searchRange = GridRangePt(Class, Index);

	//Check which values (1 through 9) appear in search range
	bool ValUsed[9] = { 0 };//an indicator array. indicate if each of numbers 1 through 9 appear in search range
	int currentVal;
	for (int i = 0; i < 9; i++) {
		currentVal = searchRange.at(i)->GetValue();
		if (currentVal != 0)
			ValUsed[currentVal - 1] = 1;
	}

	//Update temp structure
	for (int N = 1; N <= 9; N++) {
		if (ValUsed[N - 1] == 0) {
			temp.values[temp.numValues] = N;
			temp.numValues++;
		}
	}

	return temp;
}
bool IsValidPuzzle() {
//Function that checks for any inconsistencies in a given puzzle
	bool temp = true;
	int cellVal;
	for (int row = 0;row < 9;row++) {
		for (int col = 0; col < 9; col++) {
			cellVal = Grid[row][col].GetValue();
			if (cellVal > 0) { //only check non-empty cells
				//Check if value in all cells can legally be there (based on rules of Sudoku, i.e. looking at cells that would restrict possible values)
				if (!IsPossibleVal(cellVal, PossibleValues({ row,col }))) {
					temp = false;
					break;
				}
			}
		}
		if (!temp) break;
	}
	return temp;
}
/*****************************************************************************************************/


/*****************************************************************************************************/
//05 Sudoku Algorithms: Main Algorithms
bool ContinueGuessing(); //need this function prototype for SolvePuzzle()

int SolvePuzzle() {
//Main program to solve puzzle. Returns 0 if solved successfully; 2 if inconsistency found; 1 otherwise

	//First check for inconsistencies
	if (!IsValidPuzzle()) return 2;

	//Variable declaration
	CellAddress add;
	CellPossibleValues minPV, MV;
	CellPossibleAddresses u;
	Cell * c;
	int x = 0, y = 0, N, numGuesses = 0, numRestarts = 0;
	const char Classes[3] = { 'R','C','Q' };
	char Class;
	bool newResults = false;
	int currentValType = 2, rndGuess;

SearchByCell:
	//(1)Candidate Checking Method
	add = MinPossibleValueAddress();
	if (add == AddFlag1) //Puzzle is solved
		return 0;
	else if (add == AddFlag2) //Inconsistency found
		goto Inconsistency;
	else {
		minPV = PossibleValues(add);
		c = GridPt(add);
		while (minPV.numValues == 1) {
			c->SetValue(minPV.values[0]);
			c->SetValType(currentValType);
			x++;
			add = MinPossibleValueAddress();
			minPV = PossibleValues(add);
			c = GridPt(add);
		}

		if (NumberOfEmptyCells() == 0)
			return 0;
		else
			y = x;
	}

	//(2)Place Checking Method; go through each row, column, and quadrant
	for (int i = 0; i < 3; i++) {
		Class = Classes[i];
		for (int Index = 0; Index < 9; Index++) {
			MV = MissingValues(Class, Index);
			for (int j = 0; j < MV.numValues; j++) {
				N = MV.values.at(j);
				u = PossibleCells(N, Class, Index);
				if (u.numAddresses == 1) {
					c = GridPt(u.addresses[0]);
					c->SetValue(N);
					c->SetValType(currentValType);
					x++;
				}
			}
		}
		if (x > y) {
			y = x;
			newResults = true;
			break;
		}
	}
	if (newResults) {
		newResults = false;
		goto SearchByCell;
	}

Guess:
	//(3) Guessing Method
	add = MinPossibleValueAddress();
	if (add == AddFlag2) {
	Inconsistency:
		if (numGuesses == 0) {
			//We still haven't guessed but arrived at a cell with no possible values => No solution possible
			return 2;
		}
		else {
			ClearGuesses();
			//A guess probably led to an inconsistency. So, start over(for a maximum of n=MaxGuessRestarts times) after last guess
			if (numRestarts < MaxGuessRestarts) {
				numRestarts++;
				numGuesses = 0;
				goto Guess;
			}
			else {
				if (ContinueGuessing()) {
					numRestarts = 0;
					numGuesses = 0;
					goto Guess;
				}
				else {
					return 1;
				}
			}
		}

	}
	else {
		c = GridPt(add);
		minPV = PossibleValues(add); //Since add != AddFlag2, minPV.numValues > 0
		rndGuess = rand() % minPV.numValues; //random number from 0, 1, 2, .. (minPV.numValues - 1)
		c->SetValue(minPV.values.at(rndGuess));
		currentValType = 3;//all future solved cells follow a guess and are not guaranteed to be correct
		c->SetValType(currentValType);
		numGuesses++;
		goto SearchByCell;
	}

	return 1;
}
bool CreatePuzzle(int NumEmptyCells) {
//Creates a new puzzle. Returns 0 if successful, 1 otherwise
//Must catch errors of SolvePuzzle function
	int solveResult;

	//Clear existing puzzle & set all ValTypes as starting values
	for (int row = 0;row < 9;row++) {
		for (int col = 0; col < 9; col++) {
				Grid[row][col].SetValue(0);
				Grid[row][col].SetValType(0);
		}
	}

	//Create new puzzle
	solveResult = SolvePuzzle();
	if (solveResult != 0) {
		return 1;
	}

	//Remove necessary cells
	int rndRow, rndCol;
	int n = 0;
	while (n < NumEmptyCells) {
		rndRow = rand() % 9; //random number from 0 to 8
		rndCol = rand() % 9; //random number from 0 to 8
		if (Grid[rndRow][rndCol].GetValue()>0) {
			Grid[rndRow][rndCol].SetValue(0);
			Grid[rndRow][rndCol].SetValType(0);
			n++;
		}
	}
	return 0;
}
/*****************************************************************************************************/


/*****************************************************************************************************/
//06 Device-Specific Implementation
bool ContinueGuessing() {
//When the guessing algotrithm must clear all gueses at least MaxGuessRestarts times, prompt user
//  to see if they want to keep searching for a solution
//User interaction is device-specific
	char userContinue;
	cout << "> The solver restarted " << MaxGuessRestarts << " times and a solution has not been found. Do you want to try another "
		 << MaxGuessRestarts << " times? If yes, enter 'Y'. Otherwise, enter any other character and hit enter." << endl << ">";
	cin >> userContinue;
	if (userContinue == 'Y') {
		return true;
	}
	else {
		return false;
	}
}
void PrintGrid() {
//Prints the Sudoku grid to the console
	cout << endl << endl;
	cout << " Sudoku Grid" << endl;
	cout << " __________________________" << endl << endl;
	cout << "   - - - - - - - - - - -" << endl;
	for (int row = 0;row< 9;row++) {
		for (int col = 0; col < 9; col++) {
			if (col == 0 || col == 3 || col == 6)
				cout << " | ";
			else
				cout << " ";
			cout << Grid[row][col].GetValueChar();
		}
		cout << " |" << endl;
		if (row == 2 || row == 5)
			cout << " | - - - - - - - - - - - |" << endl;
	}
	cout << "   - - - - - - - - - - -" << endl;
	cout << endl;
}
bool LoadGrid(string filepath) {
//Loads a Sudoku puzze from a file
//Returns 0 is load successful; otherwise, returns 1
	ifstream fin;
	fin.open(filepath.c_str());
	if (fin) {
		char temp = '0';
		int tempInt = 0;
		for (int row = 0;row < 9;row++) {
			for (int col = 0; col < 9; col++) {
				fin >> temp; //ignores white space (space, tab, new line)
				//cout << temp; //for debuging
				if (fin) {//to check that the last character read wasn't an eof
					tempInt = int(temp - '0'); //subtracts from temp the char equivalent to 0 
					tempInt = (tempInt >= 0)*(tempInt <= 9) ? tempInt : 0; //if non-numeric char entered, converted to 0
					Grid[row][col].SetValue(tempInt);
					Grid[row][col].SetValType(1);
				}
			}
		}
	}
	else {
		return 1; //error b/c file could not be located in the directory
	}
	fin.close();
	return 0;
}
void SolvePuzzle_DisplayResults() {
//Procedure to solve puzzle and display results to console
	//Try to Solve Puzzle
	cout << "Initializing solver..." << endl;
	clock_t timer = clock();//start timer
	int solveResult = SolvePuzzle();
	timer = clock() - timer;//stop timer
	cout << "Solver complete." << endl << endl;

	//Return Result of Solve Attempt + Time to completion
	cout << "Result: ";
	switch (solveResult) {
	case 0:
		cout << "Puzzle Solved" << endl;
		PrintGrid();
		break;
	case 1:
		cout << "Puzzle could not be solved completely";
		PrintGrid();
		cout << "Number of Remaining Cells: " << NumberOfEmptyCells() << endl << endl;
		break;
	case 2:
		cout << "Inconsistency found in puzzle" << endl;
		break;
	}
	cout << "Run Time: " << float(timer) / CLOCKS_PER_SEC << " seconds" << endl;
}
/*****************************************************************************************************/


/*****************************************************************************************************/
//07 Main Function
int main(){
	cout << " -------------------------------------" << endl;
	cout << "        WELCOME TO SUDOKU SOLVER      " << endl << endl;
	cout << "   Created by: David Tagliamonti      " << endl;
	cout << " -------------------------------------" << endl;

	//Initialize Grid[][] array
	InitializeGrid();

	//Variable Declarations
	bool boolLoadPuzzle, boolCreatePuzzle; //indicator for puzzle load/create success
	string dummyString = ""; //used for user input (to get entire line)
	char userChoice = '1'; //to hold user choice

	//Set random seed (for different puzzle creation)
	srand((unsigned int)(1+abs(time(NULL)))); 

	//Main Loop
	while (userChoice == '1' || userChoice == '2') {
		cout << endl;
		cout << "Please choose an option : " << endl;
		cout << " (1) Enter '1' to load existing puzzle (named \"puzzle.txt\")" << endl;
		cout << " (2) Enter '2' to generate new puzzle" << endl;
		cout << " (3) Enter any other character to quit"<< endl;
		while (dummyString.length()==0) {
			cout << ">";
			getline(cin, dummyString);
		}
		dummyString.length() != 1 ? userChoice = 'q' : userChoice = dummyString.at(0); //if more than 1 char, quit;
		dummyString = ""; //reset for next time we need this varible

		switch (userChoice) { //either load puzzle or create one, then solve it. if userchoice isn't 1 or 2, skip
		case '1':
			boolLoadPuzzle = LoadGrid("puzzle.txt"); //load puzzle from file
			if (boolLoadPuzzle == 0) {
				cout << "Note: puzzle loaded successfully.";
				PrintGrid();
				cout << "Number of Empty Cells: " << NumberOfEmptyCells() << endl << endl;
				cout << "Hit enter to solve puzzle..."; getline(cin,dummyString); cout << endl;
				dummyString = ""; //reset for next time we need this varible
				SolvePuzzle_DisplayResults();
			}
			else {
				cout << "Error: file not found. Please check file name in the directory." << endl;
			}
			system("pause"); //to pause before displaying instructions again
			break;
		case '2':
			cout << endl << "Creating puzzle..." << endl;
			boolCreatePuzzle = CreatePuzzle(45+rand()%(60-45+1)); //between 45 and 60 missing values (i.e. 21 to 36 clues)
			if (boolCreatePuzzle == 0) {
				cout << "Puzzle successfully created." << endl;
				PrintGrid();
				cout << "Number of Empty Cells: " << NumberOfEmptyCells() << endl << endl;
				cout << "Hit enter to solve puzzle..."; getline(cin, dummyString); cout << endl;
				dummyString = ""; //reset for next time we need this varible
				SolvePuzzle_DisplayResults();
			} 
			else {
				cout << "Error: unable to create puzzle. Please try again." << endl;
			}
			system("pause"); //to pause before displaying instructions again
			break;
		}
	}

	cout << endl;
	cout << " --------------------------------------" << endl;
	cout << "   THANK YOU FOR USING SUDOKU SOLVER   " << endl;
	cout << " --------------------------------------" << endl;
	system("pause");
	return 0;
}
/*****************************************************************************************************/
