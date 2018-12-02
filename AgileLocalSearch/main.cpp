#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <algorithm>
#include <time.h>
#include <math.h>

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

class CompareSprintUtilisation {
public:
	bool operator()(pair<Sprint, double> const& a, pair<Sprint, double> const& b) {
		return a.second > b.second;
	}
};

class Roadmap {
public:
	vector<Story> stories;
	vector<Sprint> sprints;

	map<Story, Sprint> storyToSprint;
	map<Sprint, vector<Story>> sprintToStories;

	Roadmap() {};

	Roadmap(vector<Story> stories, vector<Sprint> sprints) {
		this->stories = stories;
		this->sprints = sprints;
	}

	int storyPointsAssignedToSprint(Sprint sprint) {
		vector<Story> sprintStories = sprintToStories[sprint];

		// Sum up the story points of all the stories assigned to the sprint
		int assignedStoryPoints = 0;
		for (Story story : sprintStories) {
			assignedStoryPoints += story.storyPoints;
		}

		return assignedStoryPoints;
	}

	vector<pair<Sprint, double>> sprintUtilisation() {
		vector<pair<Sprint, double>> sprintUtilisations;

		for (pair<Sprint, vector<Story>> pair : sprintToStories) {
			Sprint sprint = pair.first;

			if (sprint.sprintNumber != -1) { // Don't check the capacity of the 'unassigned' sprint
				int assignedStoryPoints = storyPointsAssignedToSprint(sprint);
				double utilisation = (double)assignedStoryPoints / (double)sprint.sprintCapacity;

				sprintUtilisations.push_back(make_pair(sprint, utilisation));
			}
		}

		return sprintUtilisations;
	}

	void addStoryToSprint(Story story, Sprint sprint) {
		storyToSprint[story] = sprint;
		sprintToStories[sprint].push_back(story);
	}

	void removeStoryFromSprint(Story story, Sprint sprint) {
		storyToSprint.erase(storyToSprint.find(story));
		sprintToStories[sprint].erase(remove(sprintToStories[sprint].begin(), sprintToStories[sprint].end(), story), sprintToStories[sprint].end());
	}

	void moveStory(Story story, Sprint from, Sprint to) {
		removeStoryFromSprint(story, from);
		addStoryToSprint(story, to);
	}

	int calculateValue() {
		int totalValue = 0;

		for (pair<Story, Sprint> pair : storyToSprint) {
			Story story = pair.first;
			Sprint sprint = pair.second;

			if (sprint.sprintNumber != -1) // Don't add value from unassigned sprints
				totalValue += story.businessValue * sprint.sprintBonus;
		}

		return totalValue;
	}

	bool sprintCapacitiesSatisifed() {
		for (pair<Sprint, vector<Story>> pair : sprintToStories) {
			Sprint sprint = pair.first;

			if (sprint.sprintNumber != -1) { // Don't check the capacity of the 'unassigned' sprint
				int assignedStoryPoints = storyPointsAssignedToSprint(sprint);

				// Check if the sprint is overloaded
				if (!sprint.withinCapacity(assignedStoryPoints))
					// The story points assigned are not within the sprint's capacity
					return false;
			}
		}

		// All sprints are within capacity
		return true;
	}

	bool storyDependenciesSatisfied() {
		for (pair<Story, Sprint> assignment : storyToSprint) {
			Story story = assignment.first;
			Sprint assignedSprint = assignment.second;

			if (assignedSprint.sprintNumber != -1) { // Don't check an unassigned story
				for (Story dependee : story.dependencies) {
					Sprint dependeeAssignedSprint = storyToSprint[dependee];

					// Only stories that are assigned to a sprint are in the map
					if (storyToSprint.find(dependee) == storyToSprint.end()) {
						// The dependee isn't assigned to a sprint
						return false;
					} else if (dependeeAssignedSprint.sprintNumber == -1) {
						// The dependee is assigned to the special 'unassigned' sprint
						return false;
					} else {
						// Check where the story is assigned compared to its dependee
						if (assignedSprint <= dependeeAssignedSprint)
							// The story is assigned to an earlier sprint than its dependee
							return false;
					}
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

		for (pair<Sprint, vector<Story>> pair : sprintToStories) {
			Sprint sprint = pair.first;

			if (sprint.sprintNumber != -1) { // Don't check the capacity of the 'unassigned' sprint
				int assignedStoryPoints = storyPointsAssignedToSprint(sprint);

				// Check if the sprint is overloaded
				if (!sprint.withinCapacity(assignedStoryPoints))
					// The story points assigned are not within the sprint's capacity
					counter += 1;
			}
		}

		// All sprints are within capacity
		return counter;
	}

	int numberOfStoryDependenciesViolated() {
		int counter = 0;

		for (pair<Story, Sprint> assignment : storyToSprint) {
			Story story = assignment.first;
			Sprint assignedSprint = assignment.second;

			if (assignedSprint.sprintNumber != -1) { // Don't check an unassigned story
				for (Story dependee : story.dependencies) {
					Sprint dependeeAssignedSprint = storyToSprint[dependee];

					// Only stories that are assigned to a sprint are in the map
					if (storyToSprint.find(dependee) == storyToSprint.end()) {
						// The dependee isn't assigned to a sprint
						counter += 1;
					}
					else if (dependeeAssignedSprint.sprintNumber == -1) {
						// The dependee is assigned to the special 'unassigned' sprint
						counter += 1;
					}
					else {
						// Check where the story is assigned compared to its dependee
						if (assignedSprint <= dependeeAssignedSprint)
							// The story is assigned to an earlier sprint than its dependee
							counter += 1;
					}
				}
			}
		}

		// All stories have their dependees assigned to an earlier sprint
		return counter;
	}



	string printStoryRoadmap() {
		string outputString = "";

		for (pair<Story, Sprint> pair : storyToSprint) {
			Story story = pair.first;
			Sprint sprint = pair.second;

			outputString += story.toString() + "\n  >> " + sprint.toString() + "\n";
		}

		return outputString;
	}

	string printSprintRoadmap() {
		string outputString = "";

		for (pair <Sprint, vector<Story>> pair : sprintToStories) {
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

class LNS {
public:
	LNS() {};

	// Return a copy of a complete solution that has been partly destroyed
	static Roadmap destroy(Roadmap completeSolution) {
		// The percentage of stories to destroy
		double degreeOfDestruction = 0.15;

		// The absolute number of stories to destroy (always at least 1)
		int storiesToDestroy = max(1, (int)round(degreeOfDestruction * completeSolution.stories.size()));

		// A vector of sprints ordered by how utilised they are
		vector<pair<Sprint, double>> sprintUtilisations = completeSolution.sprintUtilisation();
		sort(sprintUtilisations.begin(), sprintUtilisations.end(), CompareSprintUtilisation());

		for (pair<Sprint, double> pair : sprintUtilisations) {
			Sprint sprint = pair.first;
			double utilisation = pair.second;

			cout << "Sprint " << sprint.sprintNumber << " utilised " << 100. * utilisation << "%" << endl;
		}

		// IMPLEMENT
		return completeSolution;
	}

	// Repair a partly destroyed solution to a complete solution
	static Roadmap repair(Roadmap incompleteSolution) {
		// TODO
		// - Use CPLEX to find the optimal way to repair the partly-destroyed solution?
		return incompleteSolution;
	}

	// Returns whether the temporary solution should become the new current solution
	static bool accept(Roadmap temporarySolution, Roadmap currentSolution) {
		// IMPLEMENT (replace this hill-climbing)
		return temporarySolution.calculateValue() > currentSolution.calculateValue();
	}

	static Roadmap run(Roadmap currentSolution) {
		// TODO
		// - Use a better stopping condition than n iterations (less than a threshold of improvement in the past n iterations, timer etc)
		// - Only take a feasible, complete solution to remove some of the 'best solution' checks below

		// The best solution visited so far
		Roadmap bestSolution;

		// Whether a complete, feasible solution has been visited yet
		bool haveSeenAFeasibleSolution = currentSolution.isFeasible();
		
		// The initial solution might be infeasible
		if(currentSolution.isFeasible())
			bestSolution = currentSolution;

		for (int i = 0; i < 1; ++i) {
			Roadmap temporarySolution = repair(destroy(currentSolution));

			if (accept(temporarySolution, currentSolution)) {
				currentSolution = temporarySolution;
			}

			if (!haveSeenAFeasibleSolution && temporarySolution.isFeasible()) {
				// The temporary solution is the first complete, feasible solution visited
				haveSeenAFeasibleSolution = true;
				bestSolution = temporarySolution;
			} else {
				if (temporarySolution.calculateValue() > bestSolution.calculateValue()) { // Maximisation
					bestSolution = temporarySolution;
				}
			}
		}

		return bestSolution;
	}
};

int main(int argc, char* argv[]) {
	// Seed the random number generator
	srand(time(NULL));

	// The number of full time employees able to work on tasks (to estimate the sprint velocity)
	int numberOfFTEs = 1;

	// The number of sprints available in the roadmap
	int numberOfSprints;
	// Holds the data about each sprint
	vector<Sprint> sprintData;

	// The number of stories in the product backlog
	int numberOfStories;
	// Holds the data about each user story
	vector<Story> storyData;

	int numberOfEpics;
	vector<Epic> epicData;

	switch (argc) {
	case 4:
		numberOfStories = stoi(argv[1]);
		numberOfEpics = stoi(argv[2]);
		numberOfSprints = stoi(argv[3]);

		// Generate some test data to optimise
		storyData = randomlyGenerateStories(numberOfStories, 1, 10, 1, 8);

		sprintData = randomlyGenerateSprints(numberOfSprints, 0, 8 * numberOfFTEs);
		sprintData.push_back(Sprint(-1, 0, 0)); // A special sprint representing 'unassigned'

		for (int i = 0; i < numberOfEpics; ++i) {
			epicData.push_back(Epic(i));
		}

		for (Story story : storyData) {
			epicData[randomInt(0, epicData.size() - 1)].stories.push_back(story);
		}

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

	// Local search //////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	// TODO
	// - Use CPLEX to find an intial feasible, complete solution?
	
	// Generate a complete (possibly infeasible) roadmap by assigning every story to a random sprint
	Roadmap randomRoadmap  = Roadmap(storyData, sprintData);

	for (Story story : storyData) {
		Sprint randomSprint = sprintData[randomInt(0, sprintData.size() - 1)];
		randomRoadmap.addStoryToSprint(story, randomSprint);
	}

	Roadmap bestSolution = LNS::run(randomRoadmap);

	// Pretty printing ///////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	cout << "Stories:" << endl << endl;

	for (Story story : storyData) {
		cout << story.toString() << endl;
	}

	cout << endl;

	cout << "Epics:" << endl << endl;

	for (Epic epic : epicData) {
		cout << epic.toString() << endl;
	}

	cout << endl;

	cout << "Sprints:" << endl << endl;

	for (Sprint sprint : sprintData) {
		cout << sprint.toString() << endl;
	}

	cout << endl;
	cout << "----------------------------------------------------------------" << endl;
	cout << endl;

	cout << randomRoadmap.printSprintRoadmap() << endl;

	cout << endl;
	cout << "----------------------------------------------------------------" << endl;
	cout << endl;

	cout << "Roadmap value: " << randomRoadmap.calculateValue() << endl << endl;

	cout << "Is feasible: " << boolalpha << randomRoadmap.isFeasible() << endl;
	cout << "(" << randomRoadmap.numberOfSprintCapacitiesViolated() << " sprint capacities exceeded)" << endl;
	cout << "(" << randomRoadmap.numberOfStoryDependenciesViolated() << " story dependencies unassigned)" << endl;
}