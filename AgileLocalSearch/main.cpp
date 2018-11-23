#include <iostream>
#include <time.h>
#include <string>
#include <vector>

using namespace std;

class Sprint {
public:
	int sprintNumber, sprintCapacity, sprintValueBonus;

	Sprint() {};
	
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

class Sprints {
public:
	vector<Sprint> sprintsData;

	Sprints() {};
	
	Sprints(vector<Sprint> sprintsData) {
		this->sprintsData = sprintsData;
	}

	string toString() {
		string outputString = "";

		for (int i = 0; i < this->sprintsData.size(); ++i) {
			outputString += this->sprintsData[i].toString();

			if (i < this->sprintsData.size() - 1) {
				outputString += "\n";
			}
		}

		return outputString;
	}
};

class Story {
public:
	int storyNumber, businessValue, storyPoints;
	vector<Story> dependencies;

	Story() {};
	
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

class Stories {
public:
	vector<Story> storiesData;

	Stories() {};
	
	Stories(vector<Story> storiesData) {
		this->storiesData = storiesData;
	}

	string toString() {
		string outputString = "";

		for (int i = 0; i < this->storiesData.size(); ++i) {
			outputString += this->storiesData[i].toString();

			if (i < this->storiesData.size() - 1) {
				outputString += "\n";
			}
		}

		return outputString;
	}
};

class Roadmap {
public:
	Sprints sprints;
	Stories stories;
	vector<int> assignments;

	Roadmap(Sprints sprints, Stories stories, vector<int> assignments) {
		this->sprints = sprints;
		this->stories = stories;
		this->assignments = assignments;
	}

	int computeValue() {
		int totalValue = 0;

		for (int i = 0; i < this->assignments.size(); ++i) {
			int storyNumber = i;
			int assignedSprint = this->assignments[i];

			if (assignedSprint > -1) { // Don't add a story's business value if it's not assigned to a sprint
				int storyBusinessValue = stories.storiesData[i].businessValue;
				int sprintBonus = sprints.sprintsData[assignedSprint].sprintValueBonus;

				totalValue += storyBusinessValue * sprintBonus;
			}
		}

		return totalValue;
	}

	string toString() {
		string outputString = "";

		for (int i = 0; i < this->assignments.size(); ++i) {
			int storyNumber = i;
			int assignedSprint = this->assignments[i];

			outputString += "Story " + to_string(i);;
			if (assignedSprint == -1) {
				outputString += " => No sprint";
				outputString += " (value: 0)\n";
			} else {
				outputString += " => Sprint " + to_string(assignedSprint);

				int storyBusinessValue = stories.storiesData[i].businessValue;
				int sprintBonus = sprints.sprintsData[assignedSprint].sprintValueBonus;

				outputString += " (value: " + to_string(storyBusinessValue) + " * " + to_string(sprintBonus) + " = " + to_string(storyBusinessValue * sprintBonus) + ")\n";
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
	cout << "Sprints:" << endl;
	cout << sprints.toString() << endl << endl;

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
	cout << "Stories:" << endl;
	cout << stories.toString() << endl << endl;

	/////////////////////////////////////////////////////////////////

	vector<int> assignment; 

	// -1 means the story is not assigned to any sprint
	assignment.push_back(randomInt(-1, numberOfSprints - 1));
	assignment.push_back(randomInt(-1, numberOfSprints - 1));
	assignment.push_back(randomInt(-1, numberOfSprints - 1));

	Roadmap roadmap = Roadmap(sprints, stories, assignment);
	cout << "Roadmap:" << endl;
	cout << roadmap.toString() << endl;
	cout << "Value: " << roadmap.computeValue();
}