#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <algorithm>
#include <time.h>

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

	bool operator == (const Story& other) const {
		return this->storyNumber == other.storyNumber;
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

class Roadmap {
public:
	vector<Story> stories;
	vector<Sprint> sprints;

	map<Story, Sprint> storiesToSprints;
	map<Sprint, vector<Story>> sprintsToStories;

	Roadmap() {};

	Roadmap(vector<Story> stories, vector<Sprint> sprints) {
		this->stories = stories;
		this->sprints = sprints;
	}

	string printStoryRoadmap() {
		string outputString = "";

		for (pair<Story, Sprint> pair : storiesToSprints) {
			Story story = pair.first;
			Sprint sprint = pair.second;

			outputString += story.toString() + "\n  >> " + sprint.toString() + "\n";
		}

		return outputString;
	}

	string printSprintRoadmap() {
		string outputString = "";

		for (pair <Sprint, vector<Story>> pair : sprintsToStories) {
			Sprint sprint = pair.first;
			vector<Story> sprintStories = pair.second;

			outputString += sprint.toString();

			for (Story story : sprintStories) {
				outputString += "\n  >> " + story.toString();
			}

			outputString += "\n\n";
		}

		return outputString;
	}

	void addStoryToSprint(Story story, Sprint sprint) {
		storiesToSprints[story] = sprint;
		sprintsToStories[sprint].push_back(story);
	}

	void removeStoryFromSprint(Story story, Sprint sprint) {
		storiesToSprints.erase(storiesToSprints.find(story));
		sprintsToStories[sprint].erase(remove(sprintsToStories[sprint].begin(), sprintsToStories[sprint].end(), story), sprintsToStories[sprint].end());
	}

	void moveStory(Story story, Sprint from, Sprint to) {
		removeStoryFromSprint(story, from);
		addStoryToSprint(story, to);
	}

	int calculateValue() {
		int totalValue = 0;

		for (pair<Story, Sprint> pair : storiesToSprints) {
			Story story = pair.first;
			Sprint sprint = pair.second;

			totalValue += story.businessValue * sprint.sprintBonus;
		}

		return totalValue;
	}

	bool sprintCapacitiesSatisifed() {
		for (pair<Sprint, vector<Story>> pair : sprintsToStories) {
			Sprint sprint = pair.first;
			vector<Story> sprintStories = pair.second;

			// Sum up the story points of all the stories assigned to the sprint
			int assignedStoryPoints = 0;
			for (Story story : sprintStories) {
				assignedStoryPoints += story.storyPoints;
			}

			// Check if the sprint is overloaded
			if (!sprint.withinCapacity(assignedStoryPoints))
				// The story points assigned are not within the sprint's capacity
				return false;
		}

		// All sprints are within capacity
		return true;
	}

	bool storyDependenciesSatisfied() {
		for (pair<Story, Sprint> assignment : storiesToSprints) {
			Story story = assignment.first;
			Sprint assignedSprint = assignment.second;

			for (Story dependee : story.dependencies) {
				// Only stories that are assigned to a sprint are in the map
				if (storiesToSprints.find(dependee) == storiesToSprints.end()) {
					// The dependee isn't assigned to a sprint
					return false;
				} else {
					Sprint dependeeAssignedSprint = storiesToSprints[dependee];

					// Check where the story is assigned compared to its dependee
					if (assignedSprint <= dependeeAssignedSprint)
						// The story is assigned to an earlier sprint than its dependee
						return false;
				}
			}
		}

		// All stories have their dependees assigned to an earlier sprint
		return true;
	}

	bool isFeasible() {
		return sprintCapacitiesSatisifed() && storyDependenciesSatisfied();
	}

	int numberOfSprintCapacitiesViolated() {
		int counter = 0;

		for (pair<Sprint, vector<Story>> pair : sprintsToStories) {
			Sprint sprint = pair.first;
			vector<Story> sprintStories = pair.second;

			// Sum up the story points of all the stories assigned to the sprint
			int assignedStoryPoints = 0;
			for (Story story : sprintStories) {
				assignedStoryPoints += story.storyPoints;
			}

			// Check if the sprint is overloaded
			if (!sprint.withinCapacity(assignedStoryPoints))
				// The story points assigned are not within the sprint's capacity
				counter += 1;
		}

		// All sprints are within capacity
		return counter;
	}

	int numberOfStoryDependenciesViolated() {
		int counter = 0;

		for (pair<Story, Sprint> assignment : storiesToSprints) {
			Story story = assignment.first;
			Sprint assignedSprint = assignment.second;

			for (Story dependee : story.dependencies) {
				// Only stories that are assigned to a sprint are in the map
				if (storiesToSprints.find(dependee) == storiesToSprints.end()) {
					// The dependee isn't assigned to a sprint
					counter += 1;
				}
				else {
					Sprint dependeeAssignedSprint = storiesToSprints[dependee];

					// Check where the story is assigned compared to its dependee
					if (assignedSprint <= dependeeAssignedSprint)
						// The story is assigned to an earlier sprint than its dependee
						counter += 1;
				}
			}
		}

		// All stories have their dependees assigned to an earlier sprint
		return counter;
	}
};

// Returns a random int between min and max (both inclusive) using a uniform distribution
int randomInt(int min, int max) {
	return rand() % (max - min + 1) + min;
}

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
		vector<Story> dependencies = allStories[v].dependencies;

		for (Story dependee : dependencies) {
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

		for (int j = 0; j < maxNumberOfDependencies; ++j) {
			// Pick a random story as a potential dependency of story i
			Story potentialDependee = storyData[randomInt(0, numberOfStories - 1)];

			// Check if the dependency is the same as story i
			bool isSelfLoop = potentialDependee == storyData[i];

			// Check if the dependency is already a dependency of story i
			bool isAlreadyDependency = find(storyData[i].dependencies.begin(), storyData[i].dependencies.end(), potentialDependee) != storyData[i].dependencies.end();

			// Retry with another random story
			if (isSelfLoop || isAlreadyDependency) {
				--j;
				continue;
			}

			// Add the dependency to story i
			storyData[i].dependencies.push_back(potentialDependee);

			// Check if adding the dependency created a cycle in the graph of dependencies (which makes it unsolvable)
			bool dependencyCreatesCycle = isCyclic(storyData);

			if (dependencyCreatesCycle) {
				// Remove the offending dependency
				storyData[i].dependencies.pop_back();
			}
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
	// Seed the random number generator
	srand(time(NULL));

	// The number of full time employees able to work on tasks (to estimate the sprint velocity)
	int numberOfFTEs = 5;

	// The number of sprints available in the roadmap
	int numberOfSprints;
	// Holds the data about each sprint
	vector<Sprint> sprintData;

	// The number of stories in the product backlog
	int numberOfStories;
	// Holds the data about each user story
	vector<Story> storyData;

	switch (argc) {
	case 3:
		numberOfSprints = stoi(argv[1]);
		numberOfStories = stoi(argv[2]);

		// Generate some test data to optimise
		storyData = randomlyGenerateStories(numberOfStories, 1, 10, 1, 8);
		sprintData = randomlyGenerateSprints(numberOfSprints, 0, 8 * numberOfFTEs);

		break;
	default:
		Story story0(0, 10, 5);
		Story story1(1, 8, 4, { story0 });
		Story story2(2, 6, 3, { story1 });

		storyData.push_back(story0);
		storyData.push_back(story1);
		storyData.push_back(story2);

		numberOfStories = storyData.size();

		Sprint sprint0(0, 5, 2);
		Sprint sprint1(1, 5, 1);

		sprintData.push_back(sprint0);
		sprintData.push_back(sprint1);

		numberOfSprints = sprintData.size();

		break;
	}

	for (Story story : storyData) {
		cout << story.toString() << endl;
	}

	cout << endl;

	for (Sprint sprint : sprintData) {
		cout << sprint.toString() << endl;
	}

	cout << endl;
	cout << "----------------------------------------------------------------" << endl;
	cout << endl;

	// Generate a complete (but possibly infeasible) roadmap by assigning stories to random sprints
	Roadmap randomRoadmap(storyData, sprintData);

	for (Story story : storyData) {
		Sprint randomSprint = sprintData[randomInt(0, sprintData.size() - 1)];
		randomRoadmap.addStoryToSprint(story, randomSprint);
	}

	cout << randomRoadmap.printSprintRoadmap() << endl;

	cout << endl;
	cout << "----------------------------------------------------------------" << endl;
	cout << endl;

	cout << "Roadmap value: " << randomRoadmap.calculateValue() << endl << endl;

	cout << "Is feasible: " << boolalpha << randomRoadmap.isFeasible() << endl;
	cout << "(" << randomRoadmap.numberOfSprintCapacitiesViolated() << " sprint capacities exceeded)" << endl;
	cout << "(" << randomRoadmap.numberOfStoryDependenciesViolated() << " story dependencies unassigned)" << endl;
}