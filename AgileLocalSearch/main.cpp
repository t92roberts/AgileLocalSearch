#include <iostream>
#include <time.h>
#include <string>
#include <vector>

using namespace std;

class Sprint {
public:
	int sprintNumber, sprintCapacity, sprintValueBonus;

	Sprint(int num, int cap, int bonus) {
		this->sprintNumber = num;
		this->sprintCapacity = cap;
		this->sprintValueBonus = bonus;
	}

	string toString() {
		return "Sprint " + to_string(sprintNumber) +
			" (capacity: " + to_string(sprintCapacity) +
			", bonus: " + to_string(sprintValueBonus) + ")";
	}
};

class Story {
public:
	int storyNumber, businessValue, storyPoints;
	vector<Story> dependencies;

	Story(int storyNumber, int businessValue, int storyPoints) {
		this->storyNumber = storyNumber;
		this->businessValue = businessValue;
		this->storyPoints = storyPoints;
		this->dependencies = {};
	}

	Story(int storyNumber, int businessValue, int storyPoints, vector<Story> dependencies) {
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
					dependenciesString += "Story " + to_string(this->dependencies[i].storyNumber);
				}
				else {
					dependenciesString += ", Story " + to_string(this->dependencies[i].storyNumber);
				}
			}

			return dependenciesString;
		}
		else {
			return "None";
		}
	}

	string toString() {
		return "Story " + to_string(storyNumber)
			+ " (business value: " + to_string(businessValue)
			+ ", story points: " + to_string(storyPoints)
			+ ", dependencies: " + printDependencies() + ")";
	}
};

class Sprints {
public:
	vector<Sprint> sprints;

	Sprints(vector<Sprint> sprints) {
		this->sprints = sprints;
	}

	string toString() {
		string outputString = "";

		for (int i = 0; i < this->sprints.size(); ++i) {
			outputString += this->sprints[i].toString() + "\n";
		}

		return outputString;
	}
};

class Stories {
public:
	vector<Story> stories;

	Stories(vector<Story> stories) {
		this->stories = stories;
	}

	string toString() {
		string outputString = "";

		for (int i = 0; i < this->stories.size(); ++i) {
			outputString += this->stories[i].toString() + "\n";
		}

		return outputString;
	}
};

class Roadmap {
public:
	vector<int> roadmap;

	Roadmap(vector<int> roadmap) {
		this->roadmap = roadmap;
	}

	string toString() {
		string outputString = "";

		for (int i = 0; i < this->roadmap.size(); ++i) {
			outputString += "Story " + to_string(i);;
			if (this->roadmap[i] == -1) {
				outputString += " => No sprint ";
			} else {
				outputString += " => Sprint " + to_string(this->roadmap[i]) + "\n";
			}
		}

		return outputString;
	}
};

// Returns a random int between min and max (both inclusive) using a uniform distribution
int randomInt(int min, int max) {
	return rand() % (max - min + 1) + min;
}

int main(int argc, char* argv[]) {
	// Seed the random number generator
	srand(time(NULL));

	int numberOfSprints = 2;

	vector<Sprint> sprintData;

	Sprint sprint0 = Sprint(0, 5, 2);
	Sprint sprint1 = Sprint(1, 5, 1);

	sprintData.push_back(sprint0);
	sprintData.push_back(sprint1);

	Sprints sprints = Sprints(sprintData);
	cout << sprints.toString() << endl;

	/////////////////////////////////////////////////////////////////

	int numberOfStories = 3;

	vector<Story> storyData;

	Story story0 = Story(0, 10, 5);
	Story story1 = Story(1, 8, 4, { story0 });
	Story story2 = Story(2, 6, 3, { story1 });
	
	storyData.push_back(story0);
	storyData.push_back(story1);
	storyData.push_back(story2);

	Stories stories = Stories(storyData);
	cout << stories.toString() << endl;

	/////////////////////////////////////////////////////////////////

	vector<int> solution; 

	solution.push_back(0);
	solution.push_back(1);
	solution.push_back(-1); // -1 means not assigned to any sprint

	Roadmap roadmap = Roadmap(solution);
	cout << roadmap.toString() << endl;
}