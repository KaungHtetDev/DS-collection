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

// Avoid division by zero when a group has no patients.
double safeAverage(double total, int count) {
	if (count == 0) return 0.0;
	return total / count;
}

// Print a separator line of the given width.
void printSeparator(char ch, int width) { cout << string(width, ch) << endl; }

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

// Reads the CSV file, calculates the costs, and returns the patient record array to main
PatientRecord *processDataset(string filename, int &recordCount) {

	// Open the dataset file
	ifstream inputFile(filename);

	// Check if the dataset file was opened successfully
	if (!inputFile.is_open()) {
		cout << "Error opening file." << endl;
		return nullptr;
	}

	string line;

	// Skip the first line because it contains the column headings
	getline(inputFile, line);

	recordCount = 0;

	// Count how many patient records are in the file
	while (getline(inputFile, line)) {
		recordCount++;
	}

	// Reset the file position to the beginning and skip the header again
	inputFile.clear();
	inputFile.seekg(0, ios::beg);
	getline(inputFile, line);

	// Create an array to store all patient records
	PatientRecord *records = new PatientRecord[recordCount];

	int index = 0;

	// Read each patient record from the CSV file and store the information in the array
	while (getline(inputFile, line)) {
		stringstream ss(line);
		string field;

		getline(ss, records[index].patientID, ',');
		getline(ss, field, ',');
		records[index].age = stoi(field);
		getline(ss, records[index].careType, ',');
		getline(ss, field, ',');
		records[index].lengthOfStay = stoi(field);
		getline(ss, field, ',');
		records[index].baseCostPerHour = stod(field);
		getline(ss, field, ',');
		records[index].daysVisitsPerYear = stoi(field);
		records[index].totalCost = records[index].lengthOfStay * records[index].baseCostPerHour * records[index].daysVisitsPerYear;

		index++;
	}

	inputFile.close();

	return records;
}

// Displays the patient records, costs, and care type results for a dataset
void displayDataset(PatientRecord *records, int recordCount, string filename) {

	cout << string(75, '=') << endl;
	cout << "DATASET: " << filename << endl;
	cout << string(75, '=') << endl;

	// Store the number of patients and total cost for each age group
	int ageGroupCount[5] = {0};
	double ageTotalCost[5] = {0.0};

	// Count the patients and add their total costs to the correct age group
	for (int i = 0; i < recordCount; i++) {
		int ageGroup = getAgeGroup(records[i].age);
		ageGroupCount[ageGroup]++;
		ageTotalCost[ageGroup] += records[i].totalCost;
	}

	// Display decimal values with two digits after the decimal point
	cout << fixed << setprecision(2);

	// Store the different care types and keep track of patient counts and costs for each age group
	string careTypes[10];
	int careTypeCount = 0;
	int groupCareCount[5][10] = {0};
	double groupCareTotalCost[5][10] = {0.0};

	// Group each patient by age group and care type, then update the patient count and total cost
	for (int i = 0; i < recordCount; i++) {
		int ageGroup = getAgeGroup(records[i].age);
		string careType = records[i].careType;

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
		groupCareTotalCost[ageGroup][careTypeIndex] += records[i].totalCost;
	}

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

// Full analysis of one dataset (array version):
// dataset overview, age group summary, care type summary,
// and age group x care type detail.
// Full analysis of one dataset (array version):
// dataset overview, age group summary, care type summary,
// and age group x care type detail.

void analyzeDatasetArray(PatientRecord *records, int recordCount, string datasetName, string facilityName) {
	cout << endl;
	printSeparator('=', 110);
	cout << "FULL ANALYSIS: " << datasetName << " (" << facilityName << ")" << endl;
	printSeparator('=', 110);

	// ---- Statistics arrays ----
	int ageCount[5] = {0};
	double ageCost[5] = {0.0};
	int ageDuration[5] = {0};

	int careCount[CARE_TYPE_COUNT] = {0};
	double careCost[CARE_TYPE_COUNT] = {0.0};
	int careDuration[CARE_TYPE_COUNT] = {0};

	int crossCount[5][CARE_TYPE_COUNT] = {0};
	double crossCost[5][CARE_TYPE_COUNT] = {0.0};
	int crossDuration[5][CARE_TYPE_COUNT] = {0};

	double totalBilling = 0.0;
	int totalDuration = 0;

	// ---- Single pass through the array ----
	for (int i = 0; i < recordCount; i++) {
		int ag = getAgeGroup(records[i].age);
		int ct = getCareTypeIndex(records[i].careType);
		double cost = records[i].totalCost;
		int dur = records[i].lengthOfStay;

		ageCount[ag]++;
		ageCost[ag] += cost;
		ageDuration[ag] += dur;

		careCount[ct]++;
		careCost[ct] += cost;
		careDuration[ct] += dur;

		crossCount[ag][ct]++;
		crossCost[ag][ct] += cost;
		crossDuration[ag][ct] += dur;

		totalBilling += cost;
		totalDuration += dur;
	}

	cout << fixed << setprecision(2);

	// ---- Table 1: Dataset overview ----
	cout << endl << "--- Dataset Overview ---" << endl;
	printSeparator('-', 110);
	cout << left << setw(30) << "Dataset" << ": " << datasetName << endl;
	cout << left << setw(30) << "Facility" << ": " << facilityName << endl;
	cout << left << setw(30) << "Total Patients" << ": " << recordCount << endl;
	cout << left << setw(30) << "Total Billing" << ": RM" << totalBilling << endl;
	cout << left << setw(30) << "Average Cost per Patient" << ": RM" << safeAverage(totalBilling, recordCount) << endl;
	cout << left << setw(30) << "Average Visit Duration" << ": " << safeAverage(totalDuration, recordCount) << " hours" << endl;
	printSeparator('-', 110);

	// ---- Table 2: Age group summary ----
	cout << endl << "--- Age Group Summary ---" << endl;
	printSeparator('-', 117);
	cout << left << setw(43) << "Age Group" << setw(10) << "Patients" << setw(18) << "Top Care Type" << setw(10) << "Top Count" << setw(15) << "Total Cost" << setw(13) << "Avg Cost" << setw(11) << "Avg Dur" << endl;
	printSeparator('-', 117);

	for (int i = 0; i < 5; i++) {
		string topCare = "N/A";
		int topCount = 0;

		for (int j = 0; j < CARE_TYPE_COUNT; j++) {
			if (crossCount[i][j] > topCount) {
				topCount = crossCount[i][j];
				topCare = CARE_TYPE_NAMES[j];
			}
		}

		cout << left << setw(43) << getAgeGroupName(i) << setw(10) << ageCount[i] << setw(18) << topCare << setw(10) << topCount << setw(15) << ageCost[i] << setw(13) << safeAverage(ageCost[i], ageCount[i]) << setw(11) << safeAverage(ageDuration[i], ageCount[i]) << endl;
	}
	printSeparator('-', 117);

	// ---- Table 3: Care type summary ----
	cout << endl << "--- Care Type Summary ---" << endl;
	printSeparator('-', 95);
	cout << left << setw(20) << "Care Type" << setw(10) << "Patients" << setw(16) << "Total Cost" << setw(16) << "Avg Cost" << setw(16) << "Avg Duration" << setw(14) << "% of Billing" << endl;
	printSeparator('-', 95);

	for (int j = 0; j < CARE_TYPE_COUNT; j++) {
		if (careCount[j] == 0) continue;

		double pct = 0.0;
		if (totalBilling > 0) pct = careCost[j] / totalBilling * 100.0;

		cout << left << setw(20) << CARE_TYPE_NAMES[j] << setw(10) << careCount[j] << setw(16) << careCost[j] << setw(16) << safeAverage(careCost[j], careCount[j]) << setw(16) << safeAverage(careDuration[j], careCount[j]) << setw(14) << pct << "%" << endl;
	}
	printSeparator('-', 95);
	cout << "Total Dataset Billing: RM" << totalBilling << endl;

	// ---- Table 4: Age group x care type detail ----
	cout << endl << "--- Age Group x Care Type Detail ---" << endl;

	for (int i = 0; i < 5; i++) {
		if (ageCount[i] == 0) {
			cout << endl << "Age Group: " << getAgeGroupName(i) << " -> No patients in this dataset." << endl;
			continue;
		}

		cout << endl << "Age Group: " << getAgeGroupName(i) << endl;
		printSeparator('-', 95);
		cout << left << setw(20) << "Care Type" << setw(10) << "Patients" << setw(16) << "Total Cost" << setw(16) << "Avg Cost" << setw(16) << "Avg Duration" << setw(14) << "% of Age Group" << endl;
		printSeparator('-', 95);

		for (int j = 0; j < CARE_TYPE_COUNT; j++) {
			if (crossCount[i][j] == 0) continue;

			double pct = 0.0;
			if (ageCost[i] > 0) pct = crossCost[i][j] / ageCost[i] * 100.0;

			cout << left << setw(20) << CARE_TYPE_NAMES[j] << setw(10) << crossCount[i][j] << setw(16) << crossCost[i][j] << setw(16) << safeAverage(crossCost[i][j], crossCount[i][j]) << setw(16) << safeAverage(crossDuration[i][j], crossCount[i][j]) << setw(14) << pct << "%" << endl;
		}

		printSeparator('-', 95);
		cout << "Age Group Total: " << ageCost[i] << " RM | Patients: " << ageCount[i] << endl;
	}

	cout << endl;
}

// Cross-dataset comparison (array version).
// Shows facility overview and age group x facility matrix.
void compareDatasetsArray(PatientRecord *recordsA, int countA, PatientRecord *recordsB, int countB, PatientRecord *recordsC, int countC) {
	cout << endl;
	printSeparator('=', 120);
	cout << "CROSS-DATASET COMPARISON (Array Version)" << endl;
	printSeparator('=', 120);

	PatientRecord *allRecords[3] = {recordsA, recordsB, recordsC};
	int allCounts[3] = {countA, countB, countC};
	string facilityNames[3] = {"Facility A", "Facility B", "Facility C"};

	cout << fixed << setprecision(2);

	// ---- Table A: Facility comparison ----
	cout << endl << "--- Facility Comparison ---" << endl;
	printSeparator('-', 133);
	cout << left << setw(13) << "Facility" << setw(10) << "Patients" << setw(16) << "Total Billing" << setw(18) << "Avg Cost/Patient" << setw(15) << "Avg Duration" << setw(17) << "Top Care Type" << setw(24) << "Top Age Group" << endl;
	printSeparator('-', 133);

	for (int d = 0; d < 3; d++) {
		PatientRecord *recs = allRecords[d];
		int n = allCounts[d];

		double totalBilling = 0.0;
		int totalDuration = 0;
		int ageCount[5] = {0};
		int careCount[CARE_TYPE_COUNT] = {0};

		for (int i = 0; i < n; i++) {
			totalBilling += recs[i].totalCost;
			totalDuration += recs[i].lengthOfStay;
			ageCount[getAgeGroup(recs[i].age)]++;
			careCount[getCareTypeIndex(recs[i].careType)]++;
		}

		string topCare = "N/A";
		int topCareCount = 0;
		for (int j = 0; j < CARE_TYPE_COUNT; j++) {
			if (careCount[j] > topCareCount) {
				topCareCount = careCount[j];
				topCare = CARE_TYPE_NAMES[j];
			}
		}

		string topAge = "N/A";
		int topAgeCount = 0;
		for (int i = 0; i < 5; i++) {
			if (ageCount[i] > topAgeCount) {
				topAgeCount = ageCount[i];
				topAge = getAgeGroupName(i);
			}
		}

		cout << left << setw(13) << facilityNames[d] << setw(10) << n << setw(16) << totalBilling << setw(18) << safeAverage(totalBilling, n) << setw(15) << safeAverage(totalDuration, n) << setw(17) << topCare << setw(24) << topAge << endl;
	}
	printSeparator('-', 133);

	// ---- Table B: Age group x facility matrix ----
	cout << endl << "--- Age Group x Facility Matrix (Total Cost / Patients) ---" << endl;
	printSeparator('-', 120);
	cout << left << setw(45) << "Age Group";
	for (int d = 0; d < 3; d++) cout << setw(26) << facilityNames[d];
	cout << endl;
	printSeparator('-', 120);

	double ageCostMatrix[3][5] = {0.0};
	int ageCountMatrix[3][5] = {0};

	for (int d = 0; d < 3; d++) {
		for (int i = 0; i < allCounts[d]; i++) {
			int ag = getAgeGroup(allRecords[d][i].age);
			ageCostMatrix[d][ag] += allRecords[d][i].totalCost;
			ageCountMatrix[d][ag]++;
		}
	}

	for (int i = 0; i < 5; i++) {
		cout << left << setw(45) << getAgeGroupName(i);

		for (int d = 0; d < 3; d++) {
			string cell;
			if (ageCountMatrix[d][i] == 0) {
				cell = "N/A";
			} else {
				ostringstream oss;
				oss << fixed << setprecision(2) << ageCostMatrix[d][i] << " / " << ageCountMatrix[d][i];
				cell = oss.str();
			}
			cout << setw(26) << cell;
		}
		cout << endl;
	}

	printSeparator('-', 120);
	cout << "(Format: Total Cost / Patient Count)" << endl;
	cout << endl;
}

// Structure to hold clinical insight stats for array version
struct InsightStatsArray {
	double ageCost[5] = {0.0};
	int ageCount[5] = {0};
	string careNames[10];
	int careCount[10] = {0};
	double careCost[10] = {0.0};
	int numCare = 0;
};

InsightStatsArray analyseArray(PatientRecord *records, int n) {
	InsightStatsArray stats;
	for (int i = 0; i < n; i++) {
		int ageGroup = getAgeGroup(records[i].age);
		stats.ageCost[ageGroup] += records[i].totalCost;
		stats.ageCount[ageGroup]++;

		string careType = records[i].careType;
		int careIndex = -1;
		for (int j = 0; j < stats.numCare; j++) {
			if (stats.careNames[j] == careType) {
				careIndex = j;
				break;
			}
		}
		if (careIndex == -1) {
			if (stats.numCare < 10) {
				stats.careNames[stats.numCare] = careType;
				careIndex = stats.numCare;
				stats.numCare++;
			} else {
				careIndex = 0;
			}
		}
		stats.careCount[careIndex]++;
		stats.careCost[careIndex] += records[i].totalCost;
	}
	return stats;
}

int findTopAgeGroupArray(const InsightStatsArray &stats) {
	int top = 0;
	for (int i = 1; i < 5; i++) {
		if (stats.ageCost[i] > stats.ageCost[top]) top = i;
	}
	return top;
}

int findBusiestCareTypeArray(const InsightStatsArray &stats) {
	int top = 0;
	for (int i = 1; i < stats.numCare; i++) {
		if (stats.careCount[i] > stats.careCount[top]) top = i;
	}
	return top;
}

int findCostliestCareTypeArray(const InsightStatsArray &stats) {
	int top = 0;
	for (int i = 1; i < stats.numCare; i++) {
		if (stats.careCost[i] > stats.careCost[top]) top = i;
	}
	return top;
}

void provideRecommendationsArray(InsightStatsArray stats, int topAge, int topTraffic, int topCost) {
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

void printDatasetInsightsArray(string name, const InsightStatsArray &stats) {
	int topAge = findTopAgeGroupArray(stats);
	cout << name << "\n";
	cout << "  Highest billing age group : " << getAgeGroupName(topAge) << " (" << stats.ageCost[topAge] << ")\n";
	if (stats.numCare > 0) {
		int busiest = findBusiestCareTypeArray(stats);
		int costliest = findCostliestCareTypeArray(stats);
		cout << "  Highest traffic care type : " << stats.careNames[busiest] << " (" << stats.careCount[busiest] << " patients)\n";
		cout << "  Highest cost care type    : " << stats.careNames[costliest] << " (" << stats.careCost[costliest] << ")\n";
		provideRecommendationsArray(stats, topAge, busiest, costliest);
	}
	cout << "\n";
}

void printComparisonTableArray(string names[], InsightStatsArray stats[], int numDatasets) {
	cout << string(119, '=') << "\n";
	cout << "CLINICAL COMPARISON TABLE" << "\n";
	cout << string(119, '=') << "\n";
	cout << left << setw(28) << "Facility" << setw(45) << "Highest Billing Age" << setw(18) << "Billing Cost" << setw(18) << "Top Care Type" << setw(10) << "Patients" << "\n";
	cout << string(119, '-') << "\n";
	for (int i = 0; i < numDatasets; i++) {
		int topAge = findTopAgeGroupArray(stats[i]);
		int busiest = findBusiestCareTypeArray(stats[i]);
		cout << left << setw(28) << names[i] << setw(45) << getAgeGroupName(topAge) << setw(18) << stats[i].ageCost[topAge] << setw(18) << stats[i].careNames[busiest] << setw(10) << stats[i].careCount[busiest] << "\n";
	}
}

void clinicalInsightsArray(PatientRecord *recordsA, int countA, string nameA, PatientRecord *recordsB, int countB, string nameB, PatientRecord *recordsC, int countC, string nameC, int numDatasets) {
	cout << fixed << setprecision(2);
	InsightStatsArray stats[3];
	string names[3] = {nameA, nameB, nameC};
	PatientRecord *records[3] = {recordsA, recordsB, recordsC};
	int counts[3] = {countA, countB, countC};

	cout << endl << string(85, '-') << "\n";
	cout << string(34, ' ') << "CLINICAL INSIGHTS" << "\n";
	cout << string(85, '-') << "\n";

	for (int i = 0; i < numDatasets; i++) {
		stats[i] = analyseArray(records[i], counts[i]);
		printDatasetInsightsArray(names[i], stats[i]);
	}

	if (numDatasets > 1) {
		printComparisonTableArray(names, stats, numDatasets);
	}
}

// SORTING (bubble sort, ascending)
void sortByAge(PatientRecord *records, int n, int &comparisons, int &swaps) {
	comparisons = 0;
	swaps = 0;

	for (int i = 0; i < n - 1; i++) {
		for (int j = 0; j < n - 1 - i; j++) {
			comparisons++;
			if (records[j].age > records[j + 1].age) {
				// Swap the two records
				PatientRecord temp = records[j];
				records[j] = records[j + 1];
				records[j + 1] = temp;
				swaps++;
			}
		}
	}
}

void sortByStay(PatientRecord *records, int n, int &comparisons, int &swaps) {
	comparisons = 0;
	swaps = 0;

	for (int i = 0; i < n - 1; i++) {
		for (int j = 0; j < n - 1 - i; j++) {
			comparisons++;
			if (records[j].lengthOfStay > records[j + 1].lengthOfStay) {
				PatientRecord temp = records[j];
				records[j] = records[j + 1];
				records[j + 1] = temp;
				swaps++;
			}
		}
	}
}

void sortByCost(PatientRecord *records, int n, int &comparisons, int &swaps) {
	comparisons = 0;
	swaps = 0;

	for (int i = 0; i < n - 1; i++) {
		for (int j = 0; j < n - 1 - i; j++) {
			comparisons++;
			if (records[j].totalCost > records[j + 1].totalCost) {
				PatientRecord temp = records[j];
				records[j] = records[j + 1];
				records[j + 1] = temp;
				swaps++;
			}
		}
	}
}

void sortByCareType(PatientRecord *records, int n, int &comparisons, int &swaps) {
	comparisons = 0;
	swaps = 0;

	for (int i = 0; i < n - 1; i++) {
		for (int j = 0; j < n - 1 - i; j++) {
			comparisons++;
			if (getCareTypeIndex(records[j].careType) > getCareTypeIndex(records[j + 1].careType)) {
				PatientRecord temp = records[j];
				records[j] = records[j + 1];
				records[j + 1] = temp;
				swaps++;
			}
		}
	}
}

PatientRecord *copyArray(PatientRecord *records, int n) {
	PatientRecord *copy = new PatientRecord[n];
	for (int i = 0; i < n; i++) copy[i] = records[i];
	return copy;
}

void printRecords(PatientRecord *records, int n) {
	int maxDisplayable = 10;
	if (n < maxDisplayable) maxDisplayable = n;

	cout << endl << "[Results]" << endl;
	printSeparator('-', 90);
	cout << left << setw(15) << "Patient ID" << setw(8) << "Age" << setw(20) << "Care Type" << setw(18) << "Stay (hrs)" << setw(20) << "Total Cost (RM)" << endl;
	printSeparator('-', 90);

	for (int i = 0; i < maxDisplayable; i++) {
		cout << left << setw(15) << records[i].patientID << setw(8) << records[i].age << setw(20) << records[i].careType << setw(18) << records[i].lengthOfStay << setw(20) << fixed << setprecision(2) << records[i].totalCost << endl;
	}

	if (n > maxDisplayable) cout << "... (" << n - maxDisplayable << " more records not shown) ..." << endl;
	printSeparator('-', 90);
}

void printSortResult(PatientRecord *records, int &n, int &comparisons, int &swaps, double &execTime) {
	printSeparator('-', 60);
	cout << left << setw(30) << "Records sorted" << ": " << n << endl;
	cout << left << setw(30) << "Comparisons made" << ": " << comparisons << endl;
	cout << left << setw(30) << "Swaps made" << ": " << swaps << endl;
	cout << left << setw(30) << "Execution time" << ": " << fixed << setprecision(4) << execTime << " ms" << endl;
	cout << left << setw(30) << "Time complexity (worst)" << ": O(n^2)" << endl;
	cout << left << setw(30) << "Memory usage" << ": O(1)" << endl;
	printSeparator('-', 60);
	printRecords(records, n);
	delete[] records;
}

void sortDataset(PatientRecord *records, int n, string datasetName) {
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
		PatientRecord *copy = copyArray(records, n);
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
PatientRecord *linSearchByAgeGroup(PatientRecord *records, int n, int targetGroup, int &comparisons, int &resultCount) {
	comparisons = 0;
	resultCount = 0;
	PatientRecord *matches = new PatientRecord[n];

	for (int i = 0; i < n; i++) {
		comparisons++;
		if (getAgeGroup(records[i].age) == targetGroup) {
			matches[resultCount++] = records[i];
		}
	}
	return matches;
}

PatientRecord *linSearchByCareType(PatientRecord *records, int n, string careType, int &comparisons, int &resultCount) {
	comparisons = 0;
	resultCount = 0;
	PatientRecord *matches = new PatientRecord[n];

	for (int i = 0; i < n; i++) {
		comparisons++;
		if (records[i].careType == careType) {
			matches[resultCount++] = records[i];
		}
	}
	return matches;
}

PatientRecord *linSearchByStayThreshold(PatientRecord *records, int n, int threshold, int &comparisons, int &resultCount) {
	comparisons = 0;
	resultCount = 0;
	PatientRecord *matches = new PatientRecord[n];

	for (int i = 0; i < n; i++) {
		comparisons++;
		if (records[i].lengthOfStay > threshold) {
			matches[resultCount++] = records[i];
		}
	}
	return matches;
}

// Sequential search (sorted)
PatientRecord *seqSearchByAgeGroup(PatientRecord *records, int n, int targetGroup, int &comparisons, int &resultCount) {
	comparisons = 0;
	resultCount = 0;

	int groupMin[] = {0, 18, 26, 46, 61};
	int groupMax[] = {17, 25, 45, 60, 100};
	int minAge = groupMin[targetGroup], maxAge = groupMax[targetGroup];

	PatientRecord *matches = new PatientRecord[n];

	int start = 0;
	while (start < n) {
		comparisons++;
		if (records[start].age >= minAge) break;
		start++;
	}

	while (start < n) {
		comparisons++;
		if (records[start].age > maxAge) break;
		matches[resultCount++] = records[start];
		start++;
	}
	return matches;
}

PatientRecord *seqSearchByCareType(PatientRecord *records, int n, string careType, int &comparisons, int &resultCount) {
	comparisons = 0;
	resultCount = 0;
	PatientRecord *matches = new PatientRecord[n];

	for (int i = 0; i < n; i++) {
		comparisons++;
		if (records[i].careType == careType) {
			matches[resultCount++] = records[i];
		} else if (resultCount > 0) {
			break;
		}
	}
	return matches;
}

PatientRecord *seqSearchByStayThreshold(PatientRecord *records, int n, int threshold, int &comparisons, int &resultCount) {
	comparisons = 0;
	resultCount = 0;
	PatientRecord *matches = new PatientRecord[n];

	int start = 0;
	while (start < n) {
		comparisons++;
		if (records[start].lengthOfStay > threshold) break;
		start++;
	}

	for (int i = start; i < n; i++) {
		matches[resultCount++] = records[i];
	}
	return matches;
}

void printComparisonResult(PatientRecord *matchesL, int &countL, int &comparisonsL, double execTimeL,
	PatientRecord *matchesS, int &countS, int &comparisonsS, double execTimeS,
	PatientRecord *sortedCopy) {
	printSeparator('-', 70);
	cout << left << setw(30) << "Linear (unsorted) found" << ": " << countL << endl;
	cout << left << setw(30) << "Linear comparisons" << ": " << comparisonsL << " / O(n)" << endl;
	cout << left << setw(30) << "Linear execution time" << ": " << fixed << setprecision(4) << execTimeL << " ms" << endl;
	cout << left << setw(30) << "Sequential (sorted) found" << ": " << countS << endl;
	cout << left << setw(30) << "Sequential comparisons" << ": " << comparisonsS << " / O(n) -- conditionally stops early" << endl;
	cout << left << setw(30) << "Sequential execution time" << ": " << fixed << setprecision(4) << execTimeS << " ms" << endl;
	printSeparator('-', 70);
	printRecords(matchesL, countL);
	delete[] matchesL;
	delete[] matchesS;
	delete[] sortedCopy;
}

void searchDataset(PatientRecord *records, int n, string datasetName) {
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

			PatientRecord *sortedCopy = copyArray(records, n);
			int dummyComparisons, dummySwaps;
			sortByAge(sortedCopy, n, dummyComparisons, dummySwaps);

			int comparisonsL, comparisonsS, countL, countS;
			auto t1 = high_resolution_clock::now();
			PatientRecord *matchesL = linSearchByAgeGroup(records, n, targetGroup, comparisonsL, countL);
			auto t2 = high_resolution_clock::now();
			double execTimeL = duration<double, milli>(t2 - t1).count();

			auto t3 = high_resolution_clock::now();
			PatientRecord *matchesS = seqSearchByAgeGroup(sortedCopy, n, targetGroup, comparisonsS, countS);
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

			PatientRecord *sortedCopy = copyArray(records, n);
			int dummyComparisons, dummySwaps;
			sortByCareType(sortedCopy, n, dummyComparisons, dummySwaps);

			int comparisonsL, comparisonsS, countL, countS;
			auto t1 = high_resolution_clock::now();
			PatientRecord *matchesL = linSearchByCareType(records, n, chosenCare, comparisonsL, countL);
			auto t2 = high_resolution_clock::now();
			double execTimeL = duration<double, milli>(t2 - t1).count();

			auto t3 = high_resolution_clock::now();
			PatientRecord *matchesS = seqSearchByCareType(sortedCopy, n, chosenCare, comparisonsS, countS);
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

			PatientRecord *sortedCopy = copyArray(records, n);
			int dummyComparisons, dummySwaps;
			sortByStay(sortedCopy, n, dummyComparisons, dummySwaps);

			int comparisonsL, comparisonsS, countL, countS;
			auto t1 = high_resolution_clock::now();
			PatientRecord *matchesL = linSearchByStayThreshold(records, n, threshold, comparisonsL, countL);
			auto t2 = high_resolution_clock::now();
			double execTimeL = duration<double, milli>(t2 - t1).count();

			auto t3 = high_resolution_clock::now();
			PatientRecord *matchesS = seqSearchByStayThreshold(sortedCopy, n, threshold, comparisonsS, countS);
			auto t4 = high_resolution_clock::now();
			double execTimeS = duration<double, milli>(t4 - t3).count();

			cout << endl << "[Search: Stay > " << threshold << " hours]" << endl;
			printComparisonResult(matchesL, countL, comparisonsL, execTimeL, matchesS, countS, comparisonsS, execTimeS, sortedCopy);
		}
	} while (searchType != 4);
}

int main() {
	int countA = 0, countB = 0, countC = 0;
	PatientRecord *recordsA = processDataset("dataset1_facility_a.csv", countA);
	PatientRecord *recordsB = processDataset("dataset2_facility_b.csv", countB);
	PatientRecord *recordsC = processDataset("dataset3_facility_c.csv", countC);

	int choice;

	// Main menu
	do {
		cout << endl;
		cout << "===== MAIN MENU (Array Version) =====" << endl;
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
					displayDataset(recordsA, countA, "dataset1_facility_a.csv");
					break;

				case 2:
					displayDataset(recordsB, countB, "dataset2_facility_b.csv");
					break;

				case 3:
					displayDataset(recordsC, countC, "dataset3_facility_c.csv");
					break;

				case 4:
					break;

				default:
					cout << "Invalid choice. Please enter a number from 1 - 4." << endl;
					break;
				}

			} while (datasetChoice != 4);

			break;
		case 2: // Full analysis menu
			int analysisChoice;
			do {
				cout << endl;
				cout << "===== ANALYSIS MENU (ARRAY) =====" << endl;
				cout << "1. Full analysis - Dataset 1 (Facility A)" << endl;
				cout << "2. Full analysis - Dataset 2 (Facility B)" << endl;
				cout << "3. Full analysis - Dataset 3 (Facility C)" << endl;
				cout << "4. Compare all datasets" << endl;
				cout << "5. Return to main menu" << endl;
				cout << "Select choice: ";
				cin >> analysisChoice;

				if (cin.fail()) {
					cin.clear();
					cin.ignore(1000, '\n');
					cout << "Invalid input. Please enter 1 - 5." << endl;
					continue;
				}

				switch (analysisChoice) {
				case 1: {
					auto startExpA = high_resolution_clock::now();
					analyzeDatasetArray(recordsA, countA, "Dataset 1", "Facility A");
					auto endExpA = high_resolution_clock::now();
					double t = duration<double, milli>(endExpA - startExpA).count();
					cout << "Array Expenditure A: " << t << " ms" << endl;
					break;
				}
				case 2: {
					auto startExpB = high_resolution_clock::now();
					analyzeDatasetArray(recordsB, countB, "Dataset 2", "Facility B");
					auto endExpB = high_resolution_clock::now();
					double t = duration<double, milli>(endExpB - startExpB).count();
					cout << "Array Expenditure B: " << t << " ms" << endl;
					break;
				}
				case 3: {
					auto startExpC = high_resolution_clock::now();
					analyzeDatasetArray(recordsC, countC, "Dataset 3", "Facility C");
					auto endExpC = high_resolution_clock::now();
					double t = duration<double, milli>(endExpC - startExpC).count();
					cout << "Array Expenditure C: " << t << " ms" << endl;
					break;
				}
				case 4: {
					auto startExpCompare = high_resolution_clock::now();
					compareDatasetsArray(recordsA, countA, recordsB, countB, recordsC, countC);
					auto endExpCompare = high_resolution_clock::now();
					double t = duration<double, milli>(endExpCompare - startExpCompare).count();
					cout << "Array Expenditure Compare: " << t << " ms" << endl;
					break;
				}
				case 5:
					break;
				default:
					cout << "Invalid choice. Please enter 1 - 5." << endl;
					break;
				}
			} while (analysisChoice != 5);
			break;
		case 3: // Clinical insights
		{
			int insightChoice;
			do {
				cout << "\n";
				cout << "===== CLINICAL INSIGHTS MENU (ARRAY) =====" << "\n";
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
					auto startInsA = high_resolution_clock::now();
					clinicalInsightsArray(recordsA, countA, "Facility A", nullptr, 0, "", nullptr, 0, "", 1);
					auto endInsA = high_resolution_clock::now();
					double t = duration<double, milli>(endInsA - startInsA).count();
					cout << "Array Insight A: " << t << " ms" << endl;
					break;
				}
				case 2: {
					auto startInsB = high_resolution_clock::now();
					clinicalInsightsArray(recordsB, countB, "Facility B", nullptr, 0, "", nullptr, 0, "", 1);
					auto endInsB = high_resolution_clock::now();
					double t = duration<double, milli>(endInsB - startInsB).count();
					cout << "Array Insight B: " << t << " ms" << endl;
					break;
				}
				case 3: {
					auto startInsC = high_resolution_clock::now();
					clinicalInsightsArray(recordsC, countC, "Facility C", nullptr, 0, "", nullptr, 0, "", 1);
					auto endInsC = high_resolution_clock::now();
					double t = duration<double, milli>(endInsC - startInsC).count();
					cout << "Array Insight C: " << t << " ms" << endl;
					break;
				}
				case 4: {
					auto startInsCompare = high_resolution_clock::now();
					clinicalInsightsArray(recordsA, countA, "Facility A", recordsB, countB, "Facility B", recordsC, countC, "Facility C", 3);
					auto endInsCompare = high_resolution_clock::now();
					double t = duration<double, milli>(endInsCompare - startInsCompare).count();
					cout << "Array Insight Compare: " << t << " ms" << endl;
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
		} break;
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
					sortDataset(recordsA, countA, "Dataset 1 - Facility A");
					break;
				case 2:
					sortDataset(recordsB, countB, "Dataset 2 - Facility B");
					break;
				case 3:
					sortDataset(recordsC, countC, "Dataset 3 - Facility C");
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
					searchDataset(recordsA, countA, "Dataset 1 - Facility A");
					break;
				case 2:
					searchDataset(recordsB, countB, "Dataset 2 - Facility B");
					break;
				case 3:
					searchDataset(recordsC, countC, "Dataset 3 - Facility C");
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

	// Free the memory used to store the patient records
	delete[] recordsA;
	delete[] recordsB;
	delete[] recordsC;

	return 0;
}
