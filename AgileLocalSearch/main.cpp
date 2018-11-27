#include <iostream>
#include <time.h>
#include <string>
#include <vector>

using namespace std;

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

	string toString() {
		return "Sprint " + to_string(sprintNumber) +
			" (capacity: " + to_string(sprintCapacity) +
			", bonus: " + to_string(sprintBonus) + ")";
	}
};

class Solution {
	// TODO /////////////////////////////////////////////////////////////////////////////////////////
	// vector<Solution> getNeighbours()
	//		- returns the list of solutions reachable by perturbing one attribute of this solution
	/////////////////////////////////////////////////////////////////////////////////////////////////

public:
	// A mapping of user stories to sprints (index = story number, value = assigned sprint number)
	vector<int> assignments;

	// Data about the user stories
	vector<Story> stories;

	// Data about the sprints
	vector<Sprint> sprints;

	Solution() {};

	Solution(vector<int> assignments, vector<Story> stories, vector<Sprint> sprints) {
		this->assignments = assignments;
		this->stories = stories;
		this->sprints = sprints;
	}

	string toString() {
		string outputString = "";

		for (int i = 0; i < assignments.size(); ++i) {
			outputString += "Story " + to_string(i) + " => ";
			
			if (assignments[i] != -1)
				outputString += "Sprint " + to_string(assignments[i]);
			else
				outputString += "None";

			if (i < assignments.size() - 1)
				outputString += "\n";
		}

		return outputString;
	}

	// Returns the total value (utility) of the solution
	int computeValue() {
		int totalValue = 0;

		for (int i = 0; i < assignments.size(); ++i) {
			if (assignments[i] != -1) { // Skip stories that are unassigned
				Story story = stories[i];
				Sprint assignedSprint = sprints[assignments[i]];

				// Add the weighted story value
				totalValue += story.businessValue * assignedSprint.sprintBonus;
			}
		}

		return totalValue;
	}

	// Returns if the solution is feasible (all of the constraints are satisfied)
	bool isFeasible() {
		return sprintCapacitiesSatisifed() && storyDependenciesSatisfied();
	}

	// Returns if there are overloaded sprints
	bool sprintCapacitiesSatisifed() {
		vector<int> sprintStoryPoints(sprints.size(), 0);

		// Sum up how many story points have been assigned to each sprint
		for (int i = 0; i < assignments.size(); ++i) {
			if (assignments[i] != -1) { // Skip stories that are unassigned
				Story story = stories[i];
				Sprint sprint = sprints[assignments[i]];

				sprintStoryPoints[sprint.sprintNumber] += story.storyPoints;
			}
		}

		// Check the number of story points against the sprint capacities
		for (int i = 0; i < sprintStoryPoints.size(); ++i) {
			int assignedStoryPoints = sprintStoryPoints[i];
			Sprint sprint = sprints[i];

			if (sprint.withinCapacity(assignedStoryPoints)) // If the number of story points is within the capacity of the sprint
				return true;
		}

		return false;
	}

	// Returns if there are misassigned dependencies (they are either not assigned to any sprint or are assigned after the dependent story)
	bool storyDependenciesSatisfied() {
		for (int i = 0; i < assignments.size(); ++i) {
			if (assignments[i] != -1) { // Skip stories that are unassigned
				Story story = stories[i];

				// Check that each of the story's dependencies have been assigned to an earlier sprint
				for (int j = 0; j < story.dependencies.size(); ++j) {
					Story dependee = story.dependencies[j];

					int dependeesAssignedSprint = assignments[dependee.storyNumber];

					if (dependeesAssignedSprint == -1) // If the dependee is unassigned
						return false;
					else if (assignments[i] <= dependeesAssignedSprint) // If the dependee is assigned after/in the same sprint as the story
						return false;
				}
			}
		}

		return true;
	}

	// Returns the number of overloaded sprints
	int numberOfOverloadedSprints() {
		vector<int> sprintStoryPoints(sprints.size(), 0);

		// Sum up how many story points have been assigned to each sprint
		for (int i = 0; i < assignments.size(); ++i) {
			if (assignments[i] != -1) { // Skip stories that are unassigned
				Story story = stories[i];
				Sprint sprint = sprints[assignments[i]];

				sprintStoryPoints[sprint.sprintNumber] += story.storyPoints;
			}
		}

		int overloadedSprints = 0;

		// Check the number of story points against the sprint capacities
		for (int i = 0; i < sprintStoryPoints.size(); ++i) {
			int assignedStoryPoints = sprintStoryPoints[i];
			Sprint sprint = sprints[i];

			if (!sprint.withinCapacity(assignedStoryPoints)) // If the number of story points is within the capacity of the sprint
				overloadedSprints += 1;
		}

		return overloadedSprints;
	}

	// Returns how many dependencies are misassigned (they are either not assigned to any sprint or are assigned after the dependent story)
	bool numberOfMisassignedDependencies() {
		int misassignedDependencies = 0;

		for (int i = 0; i < assignments.size(); ++i) {
			if (assignments[i] != -1) { // Skip stories that are unassigned
				Story story = stories[i];

				// Check that each of the story's dependencies have been assigned to an earlier sprint
				for (int j = 0; j < story.dependencies.size(); ++j) {
					Story dependee = story.dependencies[j];

					int dependeesAssignedSprint = assignments[dependee.storyNumber];

					if (dependeesAssignedSprint == -1) // If the dependee is unassigned
						misassignedDependencies += 1;
					else if (assignments[i] <= dependeesAssignedSprint) // If the dependee is assigned after/in the same sprint as the story
						misassignedDependencies += 1;
				}
			}
		}

		return misassignedDependencies;
	}
};

// Returns a random int between min and max (both inclusive) using a uniform distribution
int randomInt(int min, int max) {
	return rand() % (max - min + 1) + min;
}

int main(int argc, char* argv[]) {
	// Seed the random number generator
	srand(time(NULL));

	// Stories //////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////

	int numberOfStories = 3;

	vector<Story> stories;

	Story story0 = Story(0, 10, 5);
	Story story1 = Story(1, 8, 4, { story0 });
	Story story2 = Story(2, 6, 3, { story1 });
	
	stories.push_back(story0);
	stories.push_back(story1);
	stories.push_back(story2);

	cout << "Stories:" << endl;
	
	for (int i = 0; i < stories.size(); ++i) {
		cout << stories[i].toString() << endl;
	}

	cout << endl;

	// Sprints //////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////

	int numberOfSprints = 2;

	vector<Sprint> sprints;

	Sprint sprint0 = Sprint(0, 5, 2);
	Sprint sprint1 = Sprint(1, 5, 1);

	sprints.push_back(sprint0);
	sprints.push_back(sprint1);

	cout << "Sprints:" << endl;

	for (int i = 0; i < sprints.size(); ++i) {
		cout << sprints[i].toString() << endl;
	}

	cout << endl;

	// Random, complete (possibly infeasible) solution ///////////////
	/////////////////////////////////////////////////////////////////

	vector<int> assignments;

	for (int i = 0; i < stories.size(); ++i) {
		assignments.push_back(randomInt(-1, sprints.size() - 1));
	}

	Solution solution = Solution(assignments, stories, sprints);

	// Print out some debugging info ////////////////////////////////
	/////////////////////////////////////////////////////////////////

	cout << "Solution:" << endl;
	cout << solution.toString();

	cout << endl << endl;
	cout << "Total value: " << solution.computeValue();

	cout << endl << endl;
	cout << "Sprints overloaded: " << solution.numberOfOverloadedSprints() << endl;
	cout << "Dependencies misassigned: " << solution.numberOfMisassignedDependencies() << endl;
	cout << "Feasible: " << boolalpha << solution.isFeasible() << endl;
}