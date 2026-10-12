#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

using namespace std;
using namespace chrono;

// Stores the information for each patient
struct PatientRecord {
	string patientID;
	int age;
	string careType;
	int lengthOfStay;
	double baseCostPerHour;
	int daysVisitsPerYear;
	double totalCost;
};

/*Insight that stores the summarization of the clinic*/
struct InsightStats {
	// Total healthcare cost for the age group
	double ageCost[5] = {0.0};
	// Number of patients
	int ageCount[5] = {0};
	// different care types
	string careNames[10];
	// care type patients
	int careCount[10] = {0};
	// care type total cost
	double careCost[10] = {0.0};
	// different care types
	int numCare = 0;
};

/*stores the expediture and services*/
struct ExpenditureStats {
	// Total Care Type
	int careTypeCount = 0;
	// Stores the different Care Type
	string careTypes[10];
	// stores the patients total by age and care type
	int groupCareCount[5][10] = {0};
	// stores the total cost for patients group
	double groupCareCost[5][10] = {0.0};
	// total healthcare cost for the groups
	double ageTotalCost[5] = {0.0};
	// Total Visit Duration
	double ageTotalVisitDuration[5] = {0.0};
	// Total Visit Count
	int ageVisitCount[5] = {0};
};

// Node for the singly linked list. Each node holds one patient record
// and a pointer to the next node in the list.
struct Node {
	PatientRecord data;
	Node *next;
};

// Print a separator line of the given width.
void printSeparator(char ch, int width) { cout << string(width, ch) << endl; }

/* Calculates the memory usage of the linked list in bytes */
size_t listMemoryBytes(int n) { return (size_t)n * sizeof(Node); }

// This function checks the patient's age and puts them into the correct age group
int getAgeGroup(int age) {
	if (age <= 17)
		return 0;
	else if (age <= 25)
		return 1;
	else if (age <= 45)
		return 2;
	else if (age <= 60)
		return 3;
	else
		return 4;
}

const int CARE_TYPE_COUNT = 7;
const string CARE_TYPE_NAMES[CARE_TYPE_COUNT] = {"Emergency", "Outpatient", "Inpatient", "Vaccination", "Rehabilitation", "Routine Checkup", "Other"};

// Convert care type into index for sorting
int getCareTypeIndex(const string &careType) {
	for (int i = 0; i < CARE_TYPE_COUNT; i++) {
		if (CARE_TYPE_NAMES[i] == careType) return i;
	}
	return CARE_TYPE_COUNT - 1; // other
}

// Returns the age group name based on the group number
string getAgeGroupName(int group) {
	switch (group) {
	case 0:
		return "0-17 (Pediatrics & Adolescents)";
	case 1:
		return "18-25 (Young Adults / University Students)";
	case 2:
		return "26-45 (Working Adults - Early Career)";
	case 3:
		return "46-60 (Working Adults - Late Career)";
	case 4:
		return "61-100 (Senior Citizens / Geriatric Care)";
	default:
		return "Unknown";
	}
}

// Adds a new patient record to the end of the linked list.
// head is passed by reference so the caller's head pointer gets updated
// the first time a node is added. tail is kept so we can insert in O(1)
// instead of walking the whole list every time (same insertion order as
// the array version, which just reads rows in file order).
void appendNode(Node *&head, Node *&tail, const PatientRecord &record) {
	Node *newNode = new Node;
	newNode->data = record;
	newNode->next = nullptr;

	if (head == nullptr) {
		head = newNode;
		tail = newNode;
	} else {
		tail->next = newNode;
		tail = newNode;
	}
}

// Reads the CSV file, calculates the costs, and builds a singly linked list
// of patient records. Returns the head pointer of the list and, via
// recordCount, how many records were read (kept so the rest of the program
// can report counts without re-traversing the list every time).
Node *processDataset(string filename, int &recordCount) {
	// Open the dataset file
	ifstream inputFile(filename);

	// Check if the dataset file was opened successfully
	if (!inputFile.is_open()) {
		cout << "Error opening file." << endl;
		recordCount = 0;
		return nullptr;
	}

	string line;

	// Skip the first line because it contains the column headings
	getline(inputFile, line);

	Node *head = nullptr;
	Node *tail = nullptr;
	recordCount = 0;

	// Read each patient record from the CSV file and append it to the list
	while (getline(inputFile, line)) {
		stringstream ss(line);
		string field;
		PatientRecord record;

		getline(ss, record.patientID, ',');
		getline(ss, field, ',');
		record.age = stoi(field);
		getline(ss, record.careType, ',');
		getline(ss, field, ',');
		record.lengthOfStay = stoi(field);
		getline(ss, field, ',');
		record.baseCostPerHour = stod(field);
		getline(ss, field, ',');
		record.daysVisitsPerYear = stoi(field);
		record.totalCost = record.lengthOfStay * record.baseCostPerHour * record.daysVisitsPerYear;

		appendNode(head, tail, record);
		recordCount++;
	}

	inputFile.close();

	return head;
}

// Displays the patient records, costs, and care type results for a dataset
// by walking the linked list once and tallying results per age group / care type.
void displayDataset(Node *head, int recordCount, string filename) {
	cout << string(75, '=') << endl;
	cout << "DATASET: " << filename << endl;
	cout << string(75, '=') << endl;

	// Store the number of patients and total cost for each age group
	int ageGroupCount[5] = {0};
	double ageTotalCost[5] = {0.0};

	// Store the different care types and keep track of patient counts and costs for each age group
	string careTypes[10];
	int careTypeCount = 0;
	int groupCareCount[5][10] = {0};
	double groupCareTotalCost[5][10] = {0.0};

	// Walk the linked list once, updating age group totals and
	// age-group/care-type totals as we go
	Node *current = head;
	while (current != nullptr) {
		int ageGroup = getAgeGroup(current->data.age);
		string careType = current->data.careType;

		ageGroupCount[ageGroup]++;
		ageTotalCost[ageGroup] += current->data.totalCost;

		int careTypeIndex = -1;
		for (int j = 0; j < careTypeCount; j++) {
			if (careTypes[j] == careType) {
				careTypeIndex = j;
				break;
			}
		}

		if (careTypeIndex == -1) {
			careTypes[careTypeCount] = careType;
			careTypeIndex = careTypeCount;
			careTypeCount++;
		}

		groupCareCount[ageGroup][careTypeIndex]++;
		groupCareTotalCost[ageGroup][careTypeIndex] += current->data.totalCost;

		current = current->next;
	}

	// Display decimal values with two digits after the decimal point
	cout << fixed << setprecision(2);

	// Display the results for each age group, including care type counts, costs, and the most requested care type
	for (int i = 0; i < 5; i++) {
		cout << "Age group " << getAgeGroupName(i) << " care type counts:" << endl;
		cout << left << setw(30) << "Care Type" << setw(15) << "Patient Count" << setw(18) << "Total Cost" << setw(15) << "Average Cost" << endl;
		cout << string(75, '-') << endl;

		for (int j = 0; j < careTypeCount; j++) {
			double avgCost = 0;
			if (groupCareCount[i][j] > 0) {
				avgCost = groupCareTotalCost[i][j] / groupCareCount[i][j];
			}

			cout << left << setw(30) << careTypes[j] << setw(15) << groupCareCount[i][j] << setw(18) << groupCareTotalCost[i][j] << setw(15) << avgCost << endl;
		}

		cout << string(75, '-') << endl;
		cout << "Total Billing for Age group " << getAgeGroupName(i) << ": " << ageTotalCost[i] << " | Patients: " << ageGroupCount[i] << endl;

		int maxCount = 0;
		int maxIndex = -1;
		for (int j = 0; j < careTypeCount; j++) {
			if (groupCareCount[i][j] > maxCount) {
				maxCount = groupCareCount[i][j];
				maxIndex = j;
			}
		}

		if (maxIndex != -1) {
			cout << "Most requested care type for age group " << getAgeGroupName(i) << ": " << careTypes[maxIndex] << " (" << maxCount << " occurrences)" << endl;
		} else {
			cout << "Most requested care type for age group " << getAgeGroupName(i) << ": N/A" << endl;
		}
	}

	cout << endl;
}

// Frees every node in the linked list to avoid a memory leak.
void freeList(Node *head) {
	while (head != nullptr) {
		Node *temp = head;
		head = head->next;
		delete temp;
	}
}

/* Analyzes a linked list and returns statistics about the dataset
   This includes age group cost / counts and care-type counts / costs*/
InsightStats analyseList(Node *head) {
	InsightStats stats;
	Node *current = head;

	while (current != nullptr) {
		int ageGroup = getAgeGroup(current->data.age);

		stats.ageCost[ageGroup] += current->data.totalCost;
		stats.ageCount[ageGroup]++;

		int careIndex = -1;
		for (int i = 0; i < stats.numCare; i++) {
			if (stats.careNames[i] == current->data.careType) {
				careIndex = i;
				break;
			}
		}

		if (careIndex == -1) {
			if (stats.numCare < 10) {
				stats.careNames[stats.numCare] = current->data.careType;
				careIndex = stats.numCare;
				stats.numCare++;
			} else {
				careIndex = 0;
			}
		}

		stats.careCount[careIndex]++;
		stats.careCost[careIndex] += current->data.totalCost;

		current = current->next;
	}

	return stats;
}

/* Provides recommendations based on the analyzed dataset statistics */
void provideRecommendations(InsightStats stats, int topAge, int topTraffic, int topCost) {
	cout << "Recommendations:\n";

	if (topAge == 4) {
		cout << "- Allocate more resources for elderly patients.\n";
	}
	if (stats.careNames[topTraffic] == "Emergency") {
		cout << "- Increase emergency admission capacity.\n";
	} else if (stats.careNames[topTraffic] == "Outpatient") {
		cout << "- Improve outpatient scheduling.\n";
	} else if (stats.careNames[topTraffic] == "Rehabilitation") {
		cout << "- Increase rehabilitation capacity.\n";
	}
	if (stats.careNames[topCost] == "Inpatient") {
		cout << "- Improve inpatient bed allocation and planning.\n";
	}
}

/*Return which age group has the highest cost*/
int findTopAgeGroup(const InsightStats &stats) {
	int top = 0;
	for (int i = 1; i < 5; i++) {
		if (stats.ageCost[i] > stats.ageCost[top]) top = i;
	}
	return top;
}

/*return care type with the most patient and position*/
int findBusiestCareType(const InsightStats &stats) {
	int top = 0;
	for (int i = 1; i < stats.numCare; i++) {
		if (stats.careCount[i] > stats.careCount[top]) top = i;
	}
	return top;
}

/*return the care type with highest cost total*/
int findCostliestCareType(const InsightStats &stats) {
	int top = 0;
	for (int i = 1; i < stats.numCare; i++) {
		if (stats.careCost[i] > stats.careCost[top]) top = i;
	}
	return top;
}

void printCareTypeTotals(const ExpenditureStats &b) {
	cout << endl << "TOTAL MEDICAL COST BY CARE TYPE (ALL AGE GROUPS COMBINED)" << "\n";
	cout << string(70, '-') << "\n";
	cout << left << setw(20) << "Care Type" << setw(20) << "Patient Count" << setw(20) << "Total Cost ($)" << "\n";
	cout << string(70, '-') << "\n";

	for (int j = 0; j < b.careTypeCount; j++) {
		int totalCount = 0;
		double totalCost = 0.0;
		for (int g = 0; g < 5; g++) {
			totalCount += b.groupCareCount[g][j];
			totalCost += b.groupCareCost[g][j];
		}
		cout << left << setw(20) << b.careTypes[j] << setw(20) << totalCount << setw(20) << totalCost << "\n";
	}
	cout << string(70, '-') << "\n";
}

/*Print the insight for the chosen dataset*/
void printDatasetInsights(string name, const InsightStats &stats) {
	int topAge = findTopAgeGroup(stats);

	cout << name << "\n";
	cout << "  Highest billing age group : " << getAgeGroupName(topAge) << " (" << stats.ageCost[topAge] << ")\n";

	if (stats.numCare > 0) {
		int busiest = findBusiestCareType(stats);
		int costliest = findCostliestCareType(stats);

		cout << "  Highest traffic care type : " << stats.careNames[busiest] << " (" << stats.careCount[busiest] << " patients)\n";

		cout << "  Highest cost care type    : " << stats.careNames[costliest] << " (" << stats.careCost[costliest] << ")\n";

		provideRecommendations(stats, topAge, busiest, costliest);
	}

	cout << "\n";
}

ExpenditureStats analyseExpenditureBreakdown(Node *head) {
	ExpenditureStats b;

	for (Node *current = head; current != nullptr; current = current->next) {
		int ageGroup = getAgeGroup(current->data.age);
		string careType = current->data.careType;

		int careIndex = -1;
		for (int j = 0; j < b.careTypeCount; j++) {
			if (b.careTypes[j] == careType) {
				careIndex = j;
				break;
			}
		}
		if (careIndex == -1) {
			b.careTypes[b.careTypeCount] = careType;
			careIndex = b.careTypeCount;
			b.careTypeCount++;
		}

		b.groupCareCount[ageGroup][careIndex]++;
		b.groupCareCost[ageGroup][careIndex] += current->data.totalCost;
		b.ageTotalCost[ageGroup] += current->data.totalCost;
		b.ageTotalVisitDuration[ageGroup] += current->data.lengthOfStay;
		b.ageVisitCount[ageGroup]++;
	}

	return b;
}

double getDatasetTotalCost(const ExpenditureStats &b) {
	double total = 0.0;
	for (int g = 0; g < 5; g++) {
		total += b.ageTotalCost[g];
	}
	return total;
}

void printAgeGroupBreakdown(int ageGroup, const ExpenditureStats &b) {
	cout << endl;
	cout << "Age Group: " << getAgeGroupName(ageGroup) << "\n";
	cout << string(100, '-') << "\n";
	cout << left << setw(20) << "Care Type" << setw(20) << "Patient Count" << setw(20) << "Total Cost ($)" << setw(35) << "Average Cost per Patient ($)" << "\n";
	cout << string(100, '-') << "\n";

	for (int j = 0; j < b.careTypeCount; j++) {
		int count = b.groupCareCount[ageGroup][j];
		if (count > 0) {
			double totalCost = b.groupCareCost[ageGroup][j];
			double avgCost = totalCost / count;

			cout << left << setw(20) << b.careTypes[j] << left << setw(20) << count << left << setw(20) << totalCost << left << setw(35) << avgCost << "\n";
		}
	}

	cout << string(100, '-') << "\n";
	cout << left << "Total Billing for Age Group: $" << fixed << setprecision(2) << b.ageTotalCost[ageGroup] << "\n";
}

/* prints the expediture table */
void expenditureAnalysis(Node *head, string name) {
	cout << fixed << setprecision(2);

	ExpenditureStats breakdown = analyseExpenditureBreakdown(head);

	cout << endl << string(100, '=') << "\n";
	cout << "HEALTHCARE EXPENDITURE & SERVICE ANALYSIS: " << name << "\n";
	cout << string(100, '=') << "\n";

	cout << "Total Medical Billing Cost for Dataset: $" << getDatasetTotalCost(breakdown) << "\n";

	cout << "Tota Medical Cost Grouped by Care Type: ";
	printCareTypeTotals(breakdown);

	for (int g = 0; g < 5; g++) {
		printAgeGroupBreakdown(g, breakdown);
	}
}

void printExpenditureComparisonTable(Node *facilityA, Node *facilityB, Node *facilityC, string facilityNameA, string facilityNameB, string facilityNameC) {
	Node *lists[3] = {facilityA, facilityB, facilityC};
	string names[3] = {facilityNameA, facilityNameB, facilityNameC};

	int numDatasets = 3;

	cout << fixed << setprecision(2);

	cout << endl << string(155, '=') << "\n";
	cout << "HEALTHCARE EXPENDITURE AND VISIT DURATION COMPARISON TABLE" << "\n";
	cout << string(155, '=') << "\n";

	cout << left << setw(30) << "Facility" << setw(20) << "Total Billing ($)" << setw(50) << "Highest Billing Age" << setw(20) << "Billing Cost ($)" << setw(15) << "Patients" << setw(20) << "Total Stay"
		 << "\n";

	cout << string(155, '-') << "\n";

	double combinedCost[5] = {0.0};
	int combinedCount[5] = {0};
	double combinedDuration[5] = {0.0};

	for (int i = 0; i < numDatasets; i++) {

		ExpenditureStats breakdown = analyseExpenditureBreakdown(lists[i]);
		InsightStats insight = analyseList(lists[i]);

		int topAge = findTopAgeGroup(insight);
		int busiestCare = findBusiestCareType(insight);

		cout << left << setw(30) << names[i] << setw(20) << getDatasetTotalCost(breakdown) << setw(50) << getAgeGroupName(topAge) << setw(20) << breakdown.ageTotalCost[topAge] << setw(15) << breakdown.groupCareCount[topAge][busiestCare] << setw(20) << breakdown.ageTotalVisitDuration[topAge] << "\n";

		for (int g = 0; g < 5; g++) {

			combinedCost[g] += breakdown.ageTotalCost[g];

			combinedDuration[g] += breakdown.ageTotalVisitDuration[g];

			for (int c = 0; c < 10; c++) {
				combinedCount[g] += breakdown.groupCareCount[g][c];
			}
		}
	}

	cout << string(155, '=') << "\n";

	int best = 0;

	for (int g = 1; g < 5; g++) {
		if (combinedCost[g] > combinedCost[best]) {
			best = g;
		}
	}

	cout << "All facilities combined, highest billing age group: " << getAgeGroupName(best) << " ($" << combinedCost[best] << " across " << combinedCount[best] << " patients)"
		 << "\n";

	cout << "\n";
	cout << "AGE GROUP EXPENDITURE AND VISIT DURATION" << "\n";
	cout << string(100, '-') << "\n";

	cout << left << setw(45) << "Age Group" << setw(20) << "Total Billing ($)" << setw(15) << "Patients" << setw(20) << "Total Stay" << setw(20) << "Average Stay"
		 << "\n";

	cout << string(100, '-') << "\n";

	for (int g = 0; g < 5; g++) {

		double averageStay = 0.0;

		if (combinedCount[g] > 0) {
			averageStay = combinedDuration[g] / combinedCount[g];
		}

		cout << left << setw(45) << getAgeGroupName(g) << setw(20) << combinedCost[g] << setw(15) << combinedCount[g] << setw(20) << combinedDuration[g] << setw(20) << averageStay << "\n";
	}

	cout << string(100, '=') << "\n";
}

/*Print the comparison table for clinical insights*/
void printComparisonTable(string names[], InsightStats stats[], int numDatasets) {
	cout << string(119, '=') << "\n";
	cout << "CLINICAL COMPARISON TABLE" << "\n";
	cout << string(119, '=') << "\n";

	cout << left << setw(28) << "Facility" << setw(45) << "Highest Billing Age" << setw(18) << "Billing Cost" << setw(18) << "Top Care Type" << setw(10) << "Patients"
		 << "\n";

	cout << string(119, '-') << "\n";

	for (int i = 0; i < numDatasets; i++) {
		int topAge = findTopAgeGroup(stats[i]);
		int busiest = findBusiestCareType(stats[i]);

		cout << left << setw(28) << names[i] << setw(45) << getAgeGroupName(topAge) << setw(18) << stats[i].ageCost[topAge] << setw(18) << stats[i].careNames[busiest] << setw(10) << stats[i].careCount[busiest] << "\n";
	}
}

/*Print the clinical insight for the datasets*/
void clinicalInsights(Node *lists[], string names[], int numDatasets) {
	cout << fixed << setprecision(2);

	InsightStats stats[3];

	cout << endl << string(85, '-') << "\n";
	cout << string(34, ' ') << "CLINICAL INSIGHTS" << "\n";
	cout << string(85, '-') << "\n";

	for (int i = 0; i < numDatasets; i++) {
		stats[i] = analyseList(lists[i]);
		printDatasetInsights(names[i], stats[i]);
	}

	if (numDatasets > 1) {
		printComparisonTable(names, stats, numDatasets);
	}
}

// SORTING (bubble sort, ascending)
void sortByAge(Node *head, int n, int &comparisons, int &swaps) {
	comparisons = 0;
	swaps = 0;
	if (head == nullptr) return;

	for (int i = 0; i < n - 1; i++) {
		Node *cur = head;
		for (int j = 0; j < n - 1 - i; j++) {
			comparisons++;
			if (cur->data.age > cur->next->data.age) {
				PatientRecord temp = cur->data;
				cur->data = cur->next->data;
				cur->next->data = temp;
				swaps++;
			}
			cur = cur->next;
		}
	}
}

void sortByStay(Node *head, int n, int &comparisons, int &swaps) {
	comparisons = 0;
	swaps = 0;
	if (head == nullptr) return;

	for (int i = 0; i < n - 1; i++) {
		Node *cur = head;
		for (int j = 0; j < n - 1 - i; j++) {
			comparisons++;
			if (cur->data.lengthOfStay > cur->next->data.lengthOfStay) {
				PatientRecord temp = cur->data;
				cur->data = cur->next->data;
				cur->next->data = temp;
				swaps++;
			}
			cur = cur->next;
		}
	}
}

void sortByCost(Node *head, int n, int &comparisons, int &swaps) {
	comparisons = 0;
	swaps = 0;
	if (head == nullptr) return;

	for (int i = 0; i < n - 1; i++) {
		Node *cur = head;
		for (int j = 0; j < n - 1 - i; j++) {
			comparisons++;
			if (cur->data.totalCost > cur->next->data.totalCost) {
				PatientRecord temp = cur->data;
				cur->data = cur->next->data;
				cur->next->data = temp;
				swaps++;
			}
			cur = cur->next;
		}
	}
}

void sortByCareType(Node *head, int n, int &comparisons, int &swaps) {
	comparisons = 0;
	swaps = 0;
	if (head == nullptr) return;

	for (int i = 0; i < n - 1; i++) {
		Node *cur = head;
		for (int j = 0; j < n - 1 - i; j++) {
			comparisons++;
			if (getCareTypeIndex(cur->data.careType) > getCareTypeIndex(cur->next->data.careType)) {
				PatientRecord temp = cur->data;
				cur->data = cur->next->data;
				cur->next->data = temp;
				swaps++;
			}
			cur = cur->next;
		}
	}
}

Node *copyList(Node *head) {
	Node *newHead = nullptr;
	Node *newTail = nullptr;
	Node *cur = head;

	while (cur != nullptr) {
		Node *newNode = new Node;
		newNode->data = cur->data;
		newNode->next = nullptr;
		if (newHead == nullptr) {
			newHead = newNode;
			newTail = newNode;
		} else {
			newTail->next = newNode;
			newTail = newNode;
		}
		cur = cur->next;
	}
	return newHead;
}

void printRecords(Node *head, int n) {
	int maxDisplayable = 10;
	if (n < maxDisplayable) maxDisplayable = n;

	cout << endl << "[Results]" << endl;
	printSeparator('-', 90);
	cout << left << setw(15) << "Patient ID" << setw(8) << "Age" << setw(20) << "Care Type" << setw(18) << "Stay (hrs)" << setw(20) << "Total Cost (RM)" << endl;
	printSeparator('-', 90);

	Node *cur = head;
	int count = 0;
	while (cur != nullptr && count < maxDisplayable) {
		cout << left << setw(15) << cur->data.patientID << setw(8) << cur->data.age << setw(20) << cur->data.careType << setw(18) << cur->data.lengthOfStay << setw(20) << fixed << setprecision(2) << cur->data.totalCost << endl;
		cur = cur->next;
		count++;
	}

	if (n > maxDisplayable) cout << "... (" << n - maxDisplayable << " more records not shown) ..." << endl;
	printSeparator('-', 90);
}

void printSortResult(Node *head, int &n, int &comparisons, int &swaps, double &execTime) {
	printSeparator('-', 60);
	cout << left << setw(30) << "Records sorted" << ": " << n << endl;
	cout << left << setw(30) << "Comparisons made" << ": " << comparisons << endl;
	cout << left << setw(30) << "Swaps made" << ": " << swaps << endl;
	cout << left << setw(30) << "Execution time" << ": " << fixed << setprecision(4) << execTime << " ms" << endl;
	cout << left << setw(30) << "Time complexity (worst)" << ": O(n^2)" << endl;
	cout << left << setw(30) << "Memory usage" << ": O(n) + O(1)" << endl;
	printSeparator('-', 60);
	printRecords(head, n);
	freeList(head);
}

void sortDataset(Node *head, int n, string datasetName) {
	cout << endl;
	printSeparator('=', 80);
	cout << "SORTING DATASET: " << datasetName << endl;
	printSeparator('=', 80);

	int sortType;
	do {
		cout << endl;
		cout << "Sort by:" << endl;
		cout << "1. Age" << endl;
		cout << "2. Length of stay" << endl;
		cout << "3. Total medical cost" << endl;
		cout << "4. Return" << endl;
		cout << "Select: ";
		cin >> sortType;

		if (cin.fail() || sortType < 1 || sortType > 4) {
			cin.clear();
			cin.ignore(1000, '\n');
			cout << "Invalid input. Please enter 1 - 4." << endl;
			continue;
		}

		if (sortType == 4) break;

		int comparisons, swaps;
		Node *copy = copyList(head);
		auto tStart = high_resolution_clock::now();

		if (sortType == 1) {
			sortByAge(copy, n, comparisons, swaps);
			auto tEnd = high_resolution_clock::now();
			double execTime = duration<double, milli>(tEnd - tStart).count();

			cout << endl << "[Sorted by Age]" << endl;
			printSortResult(copy, n, comparisons, swaps, execTime);
		} else if (sortType == 2) {
			sortByStay(copy, n, comparisons, swaps);
			auto tEnd = high_resolution_clock::now();
			double execTime = duration<double, milli>(tEnd - tStart).count();

			cout << endl << "[Sorted by Length of Stay]" << endl;
			printSortResult(copy, n, comparisons, swaps, execTime);
		} else if (sortType == 3) {
			sortByCost(copy, n, comparisons, swaps);
			auto tEnd = high_resolution_clock::now();
			double execTime = duration<double, milli>(tEnd - tStart).count();

			cout << endl << "[Sorted by Total Medical Cost]" << endl;
			printSortResult(copy, n, comparisons, swaps, execTime);
		}
	} while (true);
}

// SEARCHING
// Linear search (unsorted)
Node *linSearchByAgeGroup(Node *head, int targetGroup, int &comparisons, int &resultCount) {
	comparisons = 0;
	resultCount = 0;
	Node *filteredHead = nullptr;
	Node *filteredTail = nullptr;

	Node *cur = head;
	while (cur != nullptr) {
		comparisons++;
		if (getAgeGroup(cur->data.age) == targetGroup) {
			Node *newNode = new Node;
			newNode->data = cur->data;
			newNode->next = nullptr;
			if (filteredHead == nullptr) {
				filteredHead = newNode;
				filteredTail = newNode;
			} else {
				filteredTail->next = newNode;
				filteredTail = newNode;
			}
			resultCount++;
		}
		cur = cur->next;
	}
	return filteredHead;
}

Node *linSearchByCareType(Node *head, string careType, int &comparisons, int &resultCount) {
	comparisons = 0;
	resultCount = 0;
	Node *filteredHead = nullptr;
	Node *filteredTail = nullptr;

	Node *cur = head;
	while (cur != nullptr) {
		comparisons++;
		if (cur->data.careType == careType) {
			Node *newNode = new Node;
			newNode->data = cur->data;
			newNode->next = nullptr;
			if (filteredHead == nullptr) {
				filteredHead = newNode;
				filteredTail = newNode;
			} else {
				filteredTail->next = newNode;
				filteredTail = newNode;
			}
			resultCount++;
		}
		cur = cur->next;
	}
	return filteredHead;
}

Node *linSearchByStayThreshold(Node *head, int threshold, int &comparisons, int &resultCount) {
	comparisons = 0;
	resultCount = 0;
	Node *filteredHead = nullptr;
	Node *filteredTail = nullptr;

	Node *cur = head;
	while (cur != nullptr) {
		comparisons++;
		if (cur->data.lengthOfStay > threshold) {
			Node *newNode = new Node;
			newNode->data = cur->data;
			newNode->next = nullptr;
			if (filteredHead == nullptr) {
				filteredHead = newNode;
				filteredTail = newNode;
			} else {
				filteredTail->next = newNode;
				filteredTail = newNode;
			}
			resultCount++;
		}
		cur = cur->next;
	}
	return filteredHead;
}

// Sequential search (sorted)
Node *seqSearchByAgeGroup(Node *head, int targetGroup, int &comparisons, int &resultCount) {
	comparisons = 0;
	resultCount = 0;

	int groupMin[] = {0, 18, 26, 46, 61};
	int groupMax[] = {17, 25, 45, 60, 100};
	int minAge = groupMin[targetGroup], maxAge = groupMax[targetGroup];

	Node *filteredHead = nullptr;
	Node *filteredTail = nullptr;
	Node *cur = head;

	while (cur != nullptr) {
		comparisons++;
		if (cur->data.age >= minAge) break;
		cur = cur->next;
	}

	while (cur != nullptr) {
		comparisons++;
		if (cur->data.age > maxAge) break;

		Node *newNode = new Node;
		newNode->data = cur->data;
		newNode->next = nullptr;
		if (filteredHead == nullptr) {
			filteredHead = newNode;
			filteredTail = newNode;
		} else {
			filteredTail->next = newNode;
			filteredTail = newNode;
		}
		resultCount++;
		cur = cur->next;
	}
	return filteredHead;
}

Node *seqSearchByCareType(Node *head, string careType, int &comparisons, int &resultCount) {
	comparisons = 0;
	resultCount = 0;

	Node *filteredHead = nullptr;
	Node *filteredTail = nullptr;
	Node *cur = head;

	while (cur != nullptr) {
		comparisons++;

		if (cur->data.careType == careType) {
			Node *newNode = new Node;
			newNode->data = cur->data;
			newNode->next = nullptr;

			if (filteredHead == nullptr) {
				filteredHead = newNode;
				filteredTail = newNode;
			} else {
				filteredTail->next = newNode;
				filteredTail = newNode;
			}

			resultCount++;
		} else if (resultCount > 0) {
			break; // all matching data is already traversed
		}

		cur = cur->next;
	}

	return filteredHead;
}

Node *seqSearchByStayThreshold(Node *head, int threshold, int &comparisons, int &resultCount) {
	comparisons = 0;
	resultCount = 0;

	Node *filteredHead = nullptr;
	Node *filteredTail = nullptr;
	Node *cur = head;

	while (cur != nullptr) {
		comparisons++;
		if (cur->data.lengthOfStay > threshold) break;
		cur = cur->next;
	}

	while (cur != nullptr) {
		Node *newNode = new Node;
		newNode->data = cur->data;
		newNode->next = nullptr;

		if (filteredHead == nullptr) {
			filteredHead = newNode;
			filteredTail = newNode;
		} else {
			filteredTail->next = newNode;
			filteredTail = newNode;
		}

		resultCount++;
		cur = cur->next;
	}

	return filteredHead;
}

void printComparisonResult(Node *matchesL, int &countL, int &comparisonsL, double execTimeL, Node *matchesS, int &countS, int &comparisonsS, double execTimeS, Node *sortedCopy) {
	printSeparator('-', 70);
	cout << left << setw(30) << "Linear (unsorted) found" << ": " << countL << endl;
	cout << left << setw(30) << "Linear comparisons" << ": " << comparisonsL << " / O(n)" << endl;
	cout << left << setw(30) << "Linear execution time" << ": " << fixed << setprecision(4) << execTimeL << " ms" << endl;
	cout << left << setw(30) << "Sequential (sorted) found" << ": " << countS << endl;
	cout << left << setw(30) << "Sequential comparisons" << ": " << comparisonsS << " / O(n) -- conditionally stops early" << endl;
	cout << left << setw(30) << "Sequential execution time" << ": " << fixed << setprecision(4) << execTimeS << " ms" << endl;
	printSeparator('-', 70);
	printRecords(matchesL, countL);
	freeList(matchesL);
	freeList(matchesS);
	freeList(sortedCopy);
}

void searchDataset(Node *head, int n, string datasetName) {
	cout << endl;
	printSeparator('=', 100);
	cout << "SEARCHING DATASET: " << datasetName << endl;
	printSeparator('=', 100);

	int searchType;
	do {
		cout << endl;
		cout << "Search by:" << endl;
		cout << "1. Age group" << endl;
		cout << "2. Care type" << endl;
		cout << "3. Visit duration threshold" << endl;
		cout << "4. Return" << endl;
		cout << "Select: ";
		cin >> searchType;

		if (cin.fail() || searchType < 1 || searchType > 4) {
			cin.clear();
			cin.ignore(1000, '\n');
			cout << "Invalid input. Please enter 1 - 4." << endl;
			continue;
		}

		if (searchType == 1) {
			int groupChoice;
			cout << endl << "Age groups:" << endl;
			for (int g = 0; g < 5; g++) cout << g + 1 << ". " << getAgeGroupName(g) << endl;
			cout << "Select age group (1 - 5): ";
			cin >> groupChoice;

			if (cin.fail() || groupChoice < 1 || groupChoice > 5) {
				cin.clear();
				cin.ignore(1000, '\n');
				cout << "Invalid selection." << endl;
				continue;
			}

			int targetGroup = groupChoice - 1;

			Node *sortedCopy = copyList(head);
			int dummyComparisons, dummySwaps;
			sortByAge(sortedCopy, n, dummyComparisons, dummySwaps);

			int comparisonsL, comparisonsS, countL, countS;
			auto t1 = high_resolution_clock::now();
			Node *matchesL = linSearchByAgeGroup(head, targetGroup, comparisonsL, countL);
			auto t2 = high_resolution_clock::now();
			double execTimeL = duration<double, milli>(t2 - t1).count();

			auto t3 = high_resolution_clock::now();
			Node *matchesS = seqSearchByAgeGroup(sortedCopy, targetGroup, comparisonsS, countS);
			auto t4 = high_resolution_clock::now();
			double execTimeS = duration<double, milli>(t4 - t3).count();

			cout << endl << "[Search by Age Group: " << getAgeGroupName(targetGroup) << "]" << endl;
			printComparisonResult(matchesL, countL, comparisonsL, execTimeL, matchesS, countS, comparisonsS, execTimeS, sortedCopy);
		} else if (searchType == 2) {
			int careChoice;
			cout << endl << "Care types:" << endl;
			for (int i = 0; i < CARE_TYPE_COUNT - 1; i++) cout << i + 1 << ". " << CARE_TYPE_NAMES[i] << endl;
			cout << "Select care type (1 - 6): ";
			cin >> careChoice;

			if (cin.fail() || careChoice < 1 || careChoice > 6) {
				cin.clear();
				cin.ignore(1000, '\n');
				cout << "Invalid selection." << endl;
				continue;
			}

			string chosenCare = CARE_TYPE_NAMES[careChoice - 1];

			Node *sortedCopy = copyList(head);
			int dummyComparisons, dummySwaps;
			sortByCareType(sortedCopy, n, dummyComparisons, dummySwaps);

			int comparisonsL, comparisonsS, countL, countS;
			auto t1 = high_resolution_clock::now();
			Node *matchesL = linSearchByCareType(head, chosenCare, comparisonsL, countL);
			auto t2 = high_resolution_clock::now();
			double execTimeL = duration<double, milli>(t2 - t1).count();

			auto t3 = high_resolution_clock::now();
			Node *matchesS = seqSearchByCareType(sortedCopy, chosenCare, comparisonsS, countS);
			auto t4 = high_resolution_clock::now();
			double execTimeS = duration<double, milli>(t4 - t3).count();

			cout << endl << "[Search by Care Type: " << chosenCare << "]" << endl;
			printComparisonResult(matchesL, countL, comparisonsL, execTimeL, matchesS, countS, comparisonsS, execTimeS, sortedCopy);
		} else if (searchType == 3) {
			cout << "Enter stay duration threshold in hours (e.g. 24): ";
			int threshold;
			cin >> threshold;

			if (cin.fail() || threshold < 0) {
				cin.clear();
				cin.ignore(1000, '\n');
				cout << "Invalid input." << endl;
				continue;
			}

			Node *sortedCopy = copyList(head);
			int dummyComparisons, dummySwaps;
			sortByStay(sortedCopy, n, dummyComparisons, dummySwaps);

			int comparisonsL, comparisonsS, countL, countS;
			auto t1 = high_resolution_clock::now();
			Node *matchesL = linSearchByStayThreshold(head, threshold, comparisonsL, countL);
			auto t2 = high_resolution_clock::now();
			double execTimeL = duration<double, milli>(t2 - t1).count();

			auto t3 = high_resolution_clock::now();
			Node *matchesS = seqSearchByStayThreshold(sortedCopy, threshold, comparisonsS, countS);
			auto t4 = high_resolution_clock::now();
			double execTimeS = duration<double, milli>(t4 - t3).count();

			cout << endl << "[Search: Stay > " << threshold << " hours]" << endl;
			printComparisonResult(matchesL, countL, comparisonsL, execTimeL, matchesS, countS, comparisonsS, execTimeS, sortedCopy);
		}
	} while (searchType != 4);
}

int main() {
	int countA = 0, countB = 0, countC = 0;
	Node *listA = processDataset("dataset1_facility_a.csv", countA);
	Node *listB = processDataset("dataset2_facility_b.csv", countB);
	Node *listC = processDataset("dataset3_facility_c.csv", countC);

	int choice;

	// Display the main menu and handle the user's choice
	do {
		cout << endl;
		cout << "===== MAIN MENU (Singly Linked List Version) =====" << endl;
		cout << "1. Patient demographics & billing categorization" << endl;
		cout << "2. Healthcare expenditure & service analysis" << endl;
		cout << "3. Clinical insights" << endl;
		cout << "4. Sort datasets" << endl;
		cout << "5. Search datasets" << endl;
		cout << "6. Exit" << endl;
		cout << "Enter value from 1 - 6: ";
		cin >> choice;

		if (cin.fail()) {
			cin.clear();
			cin.ignore(1000, '\n');
			cout << "Invalid input. Please enter a number from 1 - 6." << endl;
			continue;
		}

		switch (choice) {
		case 1:
			int datasetChoice;

			// Display the dataset menu and handle the user's dataset choice
			do {
				cout << endl;
				cout << "===== DATASET MENU =====" << endl;
				cout << "1. Dataset 1 - Facility A" << endl;
				cout << "2. Dataset 2 - Facility B" << endl;
				cout << "3. Dataset 3 - Facility C" << endl;
				cout << "4. Return to main menu" << endl;
				cout << "Select dataset: ";
				cin >> datasetChoice;

				if (cin.fail()) {
					cin.clear();
					cin.ignore(1000, '\n');
					cout << "Invalid input. Please enter a number from 1 - 4." << endl;
					continue;
				}

				switch (datasetChoice) {
				case 1:
					displayDataset(listA, countA, "dataset1_facility_a.csv");
					break;

				case 2:
					displayDataset(listB, countB, "dataset2_facility_b.csv");
					break;

				case 3:
					displayDataset(listC, countC, "dataset3_facility_c.csv");
					break;

				case 4:
					break;

				default:
					cout << "Invalid choice. Please enter a number from 1 - 4." << endl;
					break;
				}

			} while (datasetChoice != 4);

			break;
		case 2:
			int expenditureChoice;

			do {
				cout << "\n";
				cout << "===== EXPENDITURE ANALYSIS MENU =====" << "\n";
				cout << "1. Facility A only" << "\n";
				cout << "2. Facility B only" << "\n";
				cout << "3. Facility C only" << "\n";
				cout << "4. All datasets" << "\n";
				cout << "5. Return to main menu" << "\n";
				cout << "Select choice: ";
				cin >> expenditureChoice;

				if (cin.fail()) {
					cin.clear();
					cin.ignore(1000, '\n');
					cout << "Invalid input. Please enter 1 - 5." << "\n";
					continue;
				}

				switch (expenditureChoice) {
				case 1: {
					expenditureAnalysis(listA, "dataset1_facility_a.csv");
					break;
				}

				case 2: {
					expenditureAnalysis(listB, "dataset2_facility_b.csv");
					break;
				}

				case 3: {
					expenditureAnalysis(listC, "dataset3_facility_c.csv");
					break;
				}

				case 4: {
					printExpenditureComparisonTable(listA, listB, listC, "dataset1_facility_a.csv", "dataset2_facility_b.csv", "dataset3_facility_c.csv");
					break;
				}

				case 5: {
					break;
				}

				default: {
					cout << "Invalid choice. Please enter 1 - 5." << "\n";
					break;
				}
				}

			} while (expenditureChoice != 5);

			break;
		case 3:
			int insightChoice;
			do {
				cout << "\n";
				cout << "===== CLINICAL INSIGHTS MENU =====" << "\n";
				cout << "1. Facility A only" << "\n";
				cout << "2. Facility B only" << "\n";
				cout << "3. Facility C only" << "\n";
				cout << "4. Compare all datasets" << "\n";
				cout << "5. Return to main menu" << "\n";
				cout << "Select choice: ";
				cin >> insightChoice;

				if (cin.fail()) {
					cin.clear();
					cin.ignore(1000, '\n');
					cout << "Invalid input. Please enter 1 - 5." << "\n";
					continue;
				}

				switch (insightChoice) {
				case 1: {
					Node *singleList[1] = {listA};
					string singleName[1] = {"dataset1_facility_a.csv"};
					clinicalInsights(singleList, singleName, 1);
					break;
				}
				case 2: {
					Node *singleList[1] = {listB};
					string singleName[1] = {"dataset2_facility_b.csv"};
					clinicalInsights(singleList, singleName, 1);
					break;
				}
				case 3: {
					Node *singleList[1] = {listC};
					string singleName[1] = {"dataset3_facility_c.csv"};
					clinicalInsights(singleList, singleName, 1);
					break;
				}
				case 4: {
					Node *allLists[3] = {listA, listB, listC};
					string allNames[3] = {"dataset1_facility_a.csv", "dataset2_facility_b.csv", "dataset3_facility_c.csv"};
					clinicalInsights(allLists, allNames, 3);
					break;
				}
				case 5:
					break;
				default:
					cout << "Invalid choice. Please enter 1 - 5." << "\n";
					break;
				}
			} while (insightChoice != 5);
			break;
		case 4: // Sort datasets menu
			int sortChoice;

			do {
				cout << endl;
				cout << "===== SORT DATASETS =====" << endl;
				cout << "1. Sort dataset 1" << endl;
				cout << "2. Sort dataset 2" << endl;
				cout << "3. Sort dataset 3" << endl;
				cout << "4. Return to main menu" << endl;
				cout << "Select choice: ";
				cin >> sortChoice;

				if (cin.fail()) {
					cin.clear();
					cin.ignore(1000, '\n');
					cout << "Invalid input. Please enter 1 - 4." << endl;
					continue;
				}

				switch (sortChoice) {
				case 1:
					sortDataset(listA, countA, "Dataset 1 - Facility A");
					break;
				case 2:
					sortDataset(listB, countB, "Dataset 2 - Facility B");
					break;
				case 3:
					sortDataset(listC, countC, "Dataset 3 - Facility C");
					break;
				case 4:
					break;
				default:
					cout << "Invalid choice. Please enter 1 - 4." << endl;
					break;
				}
			} while (sortChoice != 4);

			break;
		case 5: // Search datasets menu
			int searchChoice;

			do {
				cout << endl;
				cout << "===== SEARCH DATASETS =====" << endl;
				cout << "1. Search dataset 1" << endl;
				cout << "2. Search dataset 2" << endl;
				cout << "3. Search dataset 3" << endl;
				cout << "4. Return to main menu" << endl;
				cout << "Select choice: ";
				cin >> searchChoice;

				if (cin.fail()) {
					cin.clear();
					cin.ignore(1000, '\n');
					cout << "Invalid input. Please enter 1 - 4." << endl;
					continue;
				}

				switch (searchChoice) {
				case 1:
					searchDataset(listA, countA, "Dataset 1 - Facility A");
					break;
				case 2:
					searchDataset(listB, countB, "Dataset 2 - Facility B");
					break;
				case 3:
					searchDataset(listC, countC, "Dataset 3 - Facility C");
					break;
				case 4:
					break;
				default:
					cout << "Invalid choice. Please enter 1 - 4." << endl;
					break;
				}
			} while (searchChoice != 4);

			break;
		case 6:
			cout << "Goodbye!" << endl;
			break;
		default:
			cout << "Invalid choice. Please enter a number from 1 - 6." << endl;
			break;
		}

	} while (choice != 6);

	// Free every node in each linked list to avoid a memory leak
	freeList(listA);
	freeList(listB);
	freeList(listC);

	return 0;
}
