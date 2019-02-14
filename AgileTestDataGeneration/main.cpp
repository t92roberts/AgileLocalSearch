#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <algorithm>

using namespace std;

// Returns a random int between min and max (both inclusive) using a uniform distribution
int randomInt(int min, int max) {
	return rand() % (max - min + 1) + min;
}

// Returns a random double between min and max (both inclusive) using a uniform distribution
double randomDouble(double min, double max) {
	double f = (double)rand() / RAND_MAX;
	return min + f * (max - min);
}

class Story {
public:
	int storyNumber, businessValue, storyPoints;
	vector<int> dependencies;

	Story() {};

	Story(int storyNumber, int businessValue, int storyPoints) {
		this->storyNumber = storyNumber;
		this->businessValue = businessValue;
		this->storyPoints = storyPoints;
	}

	Story(int storyNumber, int businessValue, int storyPoints, vector<int> dependencies) {
		this->storyNumber = storyNumber;
		this->businessValue = businessValue;
		this->storyPoints = storyPoints;
		this->dependencies = dependencies;
	}

	string printDependencies() {
		if (this->dependencies.size() > 0) {
			string dependenciesString = "";

			for (int i = 0; i < this->dependencies.size(); ++i) {
				if (i == 0) {
					dependenciesString += "Story " + to_string(this->dependencies[i]);
				}
				else {
					dependenciesString += ", Story " + to_string(this->dependencies[i]);
				}
			}

			return dependenciesString;
		}
		else {
			return "None";
		}
	}

	bool operator == (const Story& other) const {
		return this->storyNumber == other.storyNumber;
	}

	bool operator != (const Story& other) const {
		return this->storyNumber != other.storyNumber;
	}

	bool operator < (const Story& other) const {
		return this->storyNumber < other.storyNumber;
	}

	bool operator <= (const Story& other) const {
		return this->storyNumber <= other.storyNumber;
	}

	string toString() {
		return "Story " + to_string(storyNumber)
			+ " (business value: " + to_string(businessValue)
			+ ", story points: " + to_string(storyPoints)
			+ ", dependencies: " + printDependencies() + ")";
	}
};

class Epic {
public:
	int epicNumber;
	vector<Story> stories;

	Epic() {};

	Epic(int epicNumber) {
		this->epicNumber = epicNumber;
	}

	Epic(int epicNumber, vector<Story> stories) {
		this->epicNumber = epicNumber;
		this->stories = stories;
	}

	string printStories() {
		if (this->stories.size() > 0) {
			string storiesString = "";

			for (int i = 0; i < this->stories.size(); ++i) {
				if (i == 0) {
					storiesString += "Story " + to_string(this->stories[i].storyNumber);
				}
				else {
					storiesString += ", Story " + to_string(this->stories[i].storyNumber);
				}
			}

			return storiesString;
		}
		else {
			return "None";
		}
	}

	string toString() {
		return "Epic " + to_string(epicNumber)
			+ " (stories: " + printStories()
			+ ")";
	}
};

class Sprint {
public:
	int sprintNumber, sprintCapacity, sprintBonus;

	Sprint() {};

	Sprint(int num, int cap, int bonus) {
		this->sprintNumber = num;
		this->sprintCapacity = cap;
		this->sprintBonus = bonus;
	}

	bool withinCapacity(int storyPoints) {
		return storyPoints <= this->sprintCapacity;
	}

	bool operator == (const Sprint& other) const {
		return this->sprintNumber == other.sprintNumber;
	}

	bool operator < (const Sprint& other) const {
		return this->sprintNumber < other.sprintNumber;
	}

	bool operator <= (const Sprint& other) const {
		return this->sprintNumber <= other.sprintNumber;
	}

	string toString() {
		return "Sprint " + to_string(sprintNumber) +
			" (capacity: " + to_string(sprintCapacity) +
			", bonus: " + to_string(sprintBonus) + ")";
	}
};

// Returns a random position in the input vector according to the given probability distribution of getting each position
int randomIntDiscreteDistribution(vector<double> probabilities) {
	// Randomly generated percentage
	double randomPercentage = (double)(rand() % 1024) / 1024;

	// Threshold representing the upper limit of each probability band
	double threshold = probabilities[0];

	for (int i = 0; i < probabilities.size(); ++i) {
		// If the random percentage is within this vector position's probability threshold, return the position
		if (randomPercentage < threshold)
			return i;
		else
			// If not, extend the threshold to the next probability band
			threshold += probabilities[i + 1];
	}
}

// Uses the parametric equation of a geometric sequence to return a vector of doubles
vector<double> geometricSequence(double a, double r, double n) {
	vector<double> sequence;

	for (int i = 0; i < n; ++i) {
		sequence.push_back(a * pow(r, i));
	}

	return sequence;
}

/*
// Adapted from:
// GeeksforGeeks. (2018). Detect Cycle in a Directed Graph - GeeksforGeeks. [online] Available at: https://www.geeksforgeeks.org/detect-cycle-in-a-graph/ [Accessed 12 Nov. 2018].
// This function is a variation of DFSUytil() in https://www.geeksforgeeks.org/archives/18212
*/
// Helper function that recursively steps through the directed graph to find a cycle
bool _isCyclic(int v, bool visited[], bool *recStack, vector<Story> allStories) {
	if (!visited[v]) {
		// Mark the current story as visited and part of recursion stack
		visited[v] = true;
		recStack[v] = true;

		// Recur for all the dependencies of this story 
		vector<int> dependencies = allStories[v].dependencies;

		for (int dependeeNumber : dependencies) {
			Story dependee = allStories[dependeeNumber];
			int dependeeStoryNumber = dependee.storyNumber;
			if (!visited[dependeeStoryNumber] && _isCyclic(dependeeStoryNumber, visited, recStack, allStories))
				return true;
			else if (recStack[dependeeStoryNumber])
				return true;
		}
	}

	recStack[v] = false; // remove the story from recursion stack
	return false;
}

/*
// Adapted from:
// GeeksforGeeks. (2018). Detect Cycle in a Directed Graph - GeeksforGeeks. [online] Available at: https://www.geeksforgeeks.org/detect-cycle-in-a-graph/ [Accessed 12 Nov. 2018].
// This function is a variation of DFS() in https://www.geeksforgeeks.org/archives/18212
*/
// Returns true if the DAG of dependencies between stories has a cycle, false if not
bool isCyclic(vector <Story> allStories) {
	// Mark all the stories as not visited and not part of recursion stack
	bool *visited = new bool[allStories.size()];
	bool *recStack = new bool[allStories.size()];

	for (int i = 0; i < allStories.size(); ++i) {
		visited[i] = false;
		recStack[i] = false;
	}

	// Call the recursive helper function on each story to detect cycles in different DFS trees
	for (int i = 0; i < allStories.size(); ++i) {
		if (_isCyclic(i, visited, recStack, allStories))
			return true;
	}

	return false;
}

// Returns a vector of Story objects filled with random values
vector<Story> randomlyGenerateStories(int numberOfStories, int minBusinessValue, int maxBusinessValue, int minStoryPoints, int maxStoryPoints) {
	vector<Story> storyData;

	// Geometric sequence of probabilities for the discrete distribution random number generator
	vector<double> probabilities = geometricSequence(0.5, 0.5, (double)numberOfStories);

	// Create stories with random values
	for (int i = 0; i < numberOfStories; ++i) {
		int businessValue = randomInt(minBusinessValue, maxBusinessValue);
		int storyPoints = randomInt(minStoryPoints, maxStoryPoints);

		storyData.push_back(Story(i, businessValue, storyPoints));
	}

	for (int i = 0; i < numberOfStories; ++i) {
		// The maximum number of dependencies that story i can have
		// (can't force a specific number of dependencies as it might create cycles in the graph of dependencies between stories)
		int maxNumberOfDependencies = randomIntDiscreteDistribution(probabilities);

		// Used to randomly select stories without replacement
		vector<int> storyNumbers;

		for (int num = 0; num < numberOfStories; ++num) {
			if (num != storyData[i].storyNumber)
				storyNumbers.push_back(num);
		}

		for (int j = 0; j < maxNumberOfDependencies && storyNumbers.size() > 0; ++j) {
			// Pick a random story as a potential dependency of story i
			int potentialDependee = storyNumbers[randomInt(0, storyNumbers.size() - 1)];
			
			// Add the dependency to story i
			storyData[i].dependencies.push_back(potentialDependee);

			// Check if adding the dependency created a cycle in the graph of dependencies (which makes it unassignable)
			if (isCyclic(storyData)) {
				// Remove the offending dependency
				storyData[i].dependencies.pop_back();
			}

			// Remove the story from the list of potential dependees
			storyNumbers.erase(remove(storyNumbers.begin(), storyNumbers.end(), potentialDependee), storyNumbers.end());
		}
	}

	return storyData;
}

// Returns a vector of Sprint objects filled with random values
vector<Sprint> randomlyGenerateSprints(int numberOfSprints, int minCapacity, int maxCapacity) {
	vector<Sprint> sprintData;

	for (int i = 0; i < numberOfSprints; ++i) {
		int capacity = randomInt(minCapacity, maxCapacity);

		// Sprint(sprintNumber, sprintCapacity, sprintBonus)
		sprintData.push_back(Sprint(i, capacity, numberOfSprints - i));
	}

	return sprintData;
}

int main(int argc, char* argv[]) {
	string type;
	int dataSize;

	int minBusinessValue = 1;
	int maxBusinessValue = 10;
	int minStoryPoints = 1;
	int maxStoryPoints = 8;

	int minCapacity = 0;
	int numberOfFTEs = 5;
	int storyPointsPerFTE = 8;
	int maxCapacity = numberOfFTEs * storyPointsPerFTE;

	switch (argc) {
	case 3:
		type = argv[1];
		transform(type.begin(), type.end(), type.begin(), ::tolower);

		dataSize = stoi(argv[2]);

		break;
	case 7:
		type = argv[1];
		transform(type.begin(), type.end(), type.begin(), ::tolower);

		dataSize = stoi(argv[2]);

		if (type == "stories") {
			minBusinessValue = stoi(argv[3]);
			maxBusinessValue = stoi(argv[4]);
			minStoryPoints = stoi(argv[5]);
			maxStoryPoints = stoi(argv[6]);
		}
		else {
			cout << "Please enter: number of stories, minimum business value, maximum business value, minimum story points, maximum story points";
			exit(0);
		}

		break;
	case 5:
		type = argv[1];
		transform(type.begin(), type.end(), type.begin(), ::tolower);

		dataSize = stoi(argv[2]);

		if (type == "sprints") {
			minCapacity = stoi(argv[3]);
			maxCapacity = stoi(argv[4]);
		}
		else {
			cout << "Please enter: number of sprints, minimum capacity, maximum capacity";
			exit(0);
		}

		break;
	default:
		exit(0);
	}

	// Seed the random number generator
	srand(time(NULL));

	//cout << "Generating size " << dataSize << "..." << endl;
	
	if (type == "stories") {
		// Holds the data about each user story
		vector<Story> storyData = randomlyGenerateStories(dataSize, minBusinessValue, maxBusinessValue, minStoryPoints, maxStoryPoints);

		cout << "story_number,business_value,story_points,dependencies\n";

		for (Story story : storyData) {
			cout << story.storyNumber << "," << story.businessValue << "," << story.storyPoints;

			if (story.dependencies.size() > 0)
				cout << ",";

			for (int k = 0; k < story.dependencies.size(); ++k) {
				int dependeeNumber = story.dependencies[k];

				cout << dependeeNumber;
				if (k < story.dependencies.size() - 1)
					cout << ";";
			}

			cout << "\n";
		}
	} else if (type == "sprints") {
		// Holds the data about each sprint
		vector<Sprint> sprintData = randomlyGenerateSprints(dataSize, minCapacity, maxCapacity);

		cout << "sprint_number,sprint_capacity,sprint_bonus\n";

		for (Sprint sprint : sprintData) {
			cout << sprint.sprintNumber << "," << sprint.sprintCapacity << "," << sprint.sprintBonus << "\n";
		}
	}

	//cout << "Done" << endl;
};