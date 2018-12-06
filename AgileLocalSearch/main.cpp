#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <algorithm>
#include <time.h>
#include <math.h>
#include <random>

using namespace std;

random_device randDev;
mt19937 generator(randDev());

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

class SprintDoublePairDescending {
public:
	bool operator()(pair<Sprint, double> const& a, pair<Sprint, double> const& b) {
		return a.second > b.second;
	}
};

class StoryDoublePairDescending {
public:
	bool operator()(pair<Story, double> const& a, pair<Story, double> const& b) {
		return a.second > b.second;
	}
};

class Roadmap {
public:
	vector<Story> stories;
	vector<Epic> epics;
	vector<Sprint> sprints;

	map<Story, Sprint> storyToSprint;
	map<Story, Epic> storyToEpic;
	map<Sprint, vector<Story>> sprintToStories;

	Roadmap() {};

	Roadmap(vector<Story> stories, vector<Epic> epics, vector<Sprint> sprints) {
		this->stories = stories;
		this->epics = epics;
		this->sprints = sprints;

		for (Epic epic : epics) {
			for (Story story : epic.stories) {
				storyToEpic[story] = epic;
			}
		}
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

	vector<pair<Sprint, double>> sprintUtilisations() {
		vector<pair<Sprint, double>> sprintUtilisations;

		for (pair<Sprint, vector<Story>> pair : sprintToStories) {
			Sprint sprint = pair.first;

			if (sprint.sprintNumber != -1) { // Don't check the capacity of the 'unassigned' sprint
				int assignedStoryPoints = storyPointsAssignedToSprint(sprint);
				double utilisation = (double)assignedStoryPoints / (double)sprint.sprintCapacity;

				sprintUtilisations.push_back(make_pair(sprint, utilisation));
			}
		}

		sort(sprintUtilisations.begin(), sprintUtilisations.end(), SprintDoublePairDescending());
		return sprintUtilisations;
	}

	vector<pair<Story, double>> storyDependenciesViolated() {
		vector<pair<Story, double>> storyDependenciesViolated;

		for (pair<Story, Sprint> assignment : storyToSprint) {
			Story story = assignment.first;
			Sprint assignedSprint = assignment.second;

			int dependenciesUnassigned = 0;

			if (assignedSprint.sprintNumber != -1) { // Don't check an unassigned story
				for (Story dependee : story.dependencies) {
					Sprint dependeeAssignedSprint = storyToSprint[dependee];

					// Only stories that are assigned to a sprint are in the map
					if (storyToSprint.find(dependee) == storyToSprint.end()) {
						// The dependee isn't assigned to a sprint
						dependenciesUnassigned += 1;
					}
					else if (dependeeAssignedSprint.sprintNumber == -1) {
						// The dependee is assigned to the special 'unassigned' sprint
						dependenciesUnassigned += 1;
					}
					else {
						// Check where the story is assigned compared to its dependee
						if (assignedSprint <= dependeeAssignedSprint)
							// The story is assigned to an earlier sprint than its dependee
							dependenciesUnassigned += 1;
					}
				}
			}

			double percentUnassigned;

			// If a story has no dependencies, then it has no unassigned dependencies
			if (story.dependencies.size() == 0)
				percentUnassigned = 0;
			else
				percentUnassigned = (double)dependenciesUnassigned / (double)story.dependencies.size();

			storyDependenciesViolated.push_back(make_pair(story, percentUnassigned));
		}

		sort(storyDependenciesViolated.begin(), storyDependenciesViolated.end(), StoryDoublePairDescending());
		return storyDependenciesViolated;
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

		for (Sprint sprint : sprints) {
			if (sprint.sprintNumber == -1)
				outputString += "Unassigned";
			else
				outputString += sprint.toString();

			vector<Story> sprintStories = sprintToStories[sprint];

			if (sprintStories.empty()) {
				outputString += "\n  >> None";
			} else {
				for (Story story : sprintStories) {
					outputString += "\n  >> " + story.toString();
				}
			}

			outputString += "\n\n";
		}

		return outputString;
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

	uniform_int_distribution<int> bvDistribution(minBusinessValue, maxBusinessValue);
	uniform_int_distribution<int> spDistribution(minStoryPoints, maxStoryPoints);

	// Create stories with random values
	for (int i = 0; i < numberOfStories; ++i) {
		int businessValue = bvDistribution(generator);
		int storyPoints = spDistribution(generator);

		storyData.push_back(Story(i, businessValue, storyPoints));
	}

	uniform_int_distribution<int> dependeeDistribution(0, numberOfStories - 1);

	for (int i = 0; i < numberOfStories; ++i) {
		// The maximum number of dependencies that story i can have
		// (can't force a specific number of dependencies as it might create cycles in the graph of dependencies between stories)
		int maxNumberOfDependencies = randomIntDiscreteDistribution(probabilities);

		for (int j = 0; j < maxNumberOfDependencies; ++j) {
			// Pick a random story as a potential dependency of story i
			Story potentialDependee = storyData[dependeeDistribution(generator)];

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

	uniform_int_distribution<int> capacityDistribution(minCapacity, maxCapacity);

	for (int i = 0; i < numberOfSprints; ++i) {
		int capacity = capacityDistribution(generator);

		// Sprint(sprintNumber, sprintCapacity, sprintBonus)
		sprintData.push_back(Sprint(i, capacity, numberOfSprints - i));
	}

	return sprintData;
}

// Holds a partly-destroyed solution and a list of the stories that were removed
class DestroyedSolution {
public:
	Roadmap roadmap;
	vector<Story> removedStories;

	DestroyedSolution() {};

	DestroyedSolution(Roadmap roadmap, vector<Story> removedStories) {
		this->roadmap = roadmap;
		this->removedStories = removedStories;
	}
};

class LNS {
public:
	LNS() {};

	// Return a copy of a complete solution that has been partly destroyed
	static DestroyedSolution destroy(Roadmap completeSolution, double degreeOfDestruction) {
		// TODO
		// - Take a Tabu list as an input (to mitigate cycling)

		// The absolute number of stories to destroy (at least 1)
		int numberOfStoriesToRemove = max(1, (int)round(degreeOfDestruction * completeSolution.stories.size()));

		uniform_int_distribution<int> storyDistribution(0, completeSolution.stories.size() - 1);

		// A list of the stories removed from the solution
		vector<Story> removedStories;

		// Keep removing stories and their related stories until the required number have been removed
		while (numberOfStoriesToRemove > 0) {
			// Remove a random story
			Story randomStory = completeSolution.stories[storyDistribution(generator)];
			Sprint randomStorySprint = completeSolution.storyToSprint[randomStory];

			removedStories.push_back(randomStory);
			completeSolution.removeStoryFromSprint(randomStory, randomStorySprint);

			--numberOfStoriesToRemove;

			// Remove its dependencies
			for (int i = 0; i < randomStory.dependencies.size() && numberOfStoriesToRemove > 0; ++i) {
				Story dependee = randomStory.dependencies[i];
				Sprint dependeeSprint = completeSolution.storyToSprint[dependee];

				removedStories.push_back(dependee);
				completeSolution.removeStoryFromSprint(dependee, dependeeSprint);

				--numberOfStoriesToRemove;
			}

			vector<Story> epicSiblingStories = completeSolution.storyToEpic[randomStory].stories;

			// Remove stories in its epic
			for (int i = 0; i < epicSiblingStories.size() && numberOfStoriesToRemove > 0; ++i) {
				Story epicSiblingStory = epicSiblingStories[i];

				if (randomStory != epicSiblingStory) {
					Sprint epicSiblingStorySprint = completeSolution.storyToSprint[epicSiblingStory];

					removedStories.push_back(epicSiblingStory);
					completeSolution.removeStoryFromSprint(epicSiblingStory, epicSiblingStorySprint);

					--numberOfStoriesToRemove;
				}
			}
		}

		// A vector of stories sorted by the % of each story's dependencies are unassigned
		//vector<pair<Story, double>> storyDependenciesViolated = completeSolution.storyDependenciesViolated();

		// A vector of sprints sorted by how utilised they are (as a percentage)
		//vector<pair<Sprint, double>> sprintUtilisations = completeSolution.sprintUtilisations();

		return DestroyedSolution(completeSolution, removedStories);
	}

	// Repair a partly destroyed solution to a complete solution
	static Roadmap repair(DestroyedSolution destroyedSolution) {
		// TODO
		// - Use CPLEX to find the optimal way to repair the partly-destroyed solution?

		Roadmap roadmap = destroyedSolution.roadmap;
		vector<Story> removedStories = destroyedSolution.removedStories;

		uniform_int_distribution<int> sprintDistribution(0, roadmap.sprints.size() - 1);

		// Assign the removed stories to random sprints
		for (Story story : removedStories) {
			Sprint randomSprint = roadmap.sprints[sprintDistribution(generator)];
			roadmap.addStoryToSprint(story, randomSprint);
		}

		return roadmap;
	}

	// Returns whether the temporary solution should become the new current solution
	static bool accept(Roadmap temporarySolution, Roadmap currentSolution) {
		// TODO
		// - Don't use hill climbing
		return temporarySolution.calculateValue() >= currentSolution.calculateValue();
	}

	static Roadmap run(Roadmap currentSolution, double degreeOfDestruction, double seconds, int nonImprovingIterationsThreshold) {
		// The best solution visited so far
		Roadmap bestSolution = currentSolution;

		int nonImprovingIterations = 0;

		time_t start = time(NULL);

		while (difftime(time(NULL), start) < seconds && nonImprovingIterations < nonImprovingIterationsThreshold) {
			Roadmap temporarySolution = repair(destroy(currentSolution, degreeOfDestruction));

			if (accept(temporarySolution, currentSolution)) {
				currentSolution = temporarySolution;
			}

			if (temporarySolution.calculateValue() > bestSolution.calculateValue() && temporarySolution.isFeasible()) { // Maximisation
				bestSolution = temporarySolution;
				nonImprovingIterations = 0;
			} else {
				nonImprovingIterations += 1;
			}
		}

		return bestSolution;
	}
};

int main(int argc, char* argv[]) {
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

	// The number of epics in the product backlog
	int numberOfEpics;
	// Holds the data about each epic
	vector<Epic> epicData;

	switch (argc) {
	case 4:
	{
		numberOfStories = stoi(argv[1]);
		numberOfEpics = stoi(argv[2]);
		numberOfSprints = stoi(argv[3]);

		// Generate some test data to optimise
		storyData = randomlyGenerateStories(numberOfStories, 1, 10, 1, 8);

		// Epics
		for (int i = 0; i < numberOfEpics; ++i) {
			epicData.push_back(Epic(i));
		}

		uniform_int_distribution<int> epicDistribution(0, epicData.size() - 1);

		// Add every story to a random epic
		for (Story story : storyData) {
			epicData[epicDistribution(generator)].stories.push_back(story);
		}

		sprintData = randomlyGenerateSprints(numberOfSprints, 0, 8 * numberOfFTEs);
		sprintData.push_back(Sprint(-1, 0, 0)); // A special sprint representing 'unassigned'

		break;
	}
	default:
		Story story0(0, 10, 5);
		Story story1(1, 8, 4, { story0 });
		Story story2(2, 6, 3, { story1 });

		storyData.push_back(story0);
		storyData.push_back(story1);
		storyData.push_back(story2);

		numberOfStories = storyData.size();

		epicData.push_back(Epic(0, { story0, story1 }));
		epicData.push_back(Epic(1, { story2 }));

		Sprint sprint0(0, 5, 2);
		Sprint sprint1(1, 5, 1);

		sprintData.push_back(sprint0);
		sprintData.push_back(sprint1);
		sprintData.push_back(Sprint(-1, 0, 0)); // A special sprint representing 'unassigned'

		numberOfSprints = sprintData.size();

		break;
	}

	// Local search //////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	// TODO
	// - Use CPLEX to find an initial feasible, complete solution?
	
	// A complete, feasible roadmap
	Roadmap randomRoadmap;

	time_t initialStart = time(NULL);
	cout << "Searching for admissable initial solution..." << endl;

	// Assign every story to a random sprint until it makes a complete, feasible solution
	do {
		randomRoadmap = Roadmap(storyData, epicData, sprintData);

		uniform_int_distribution<int> sprintDistribution(0, sprintData.size() - 1);

		for (Story story : storyData) {
			Sprint randomSprint = sprintData[sprintDistribution(generator)];
			randomRoadmap.addStoryToSprint(story, randomSprint);
		}
	} while (!randomRoadmap.isFeasible());

	cout << "Initial solution found in " << difftime(time(NULL), initialStart) << " seconds" << endl << endl;

	cout << "Solving..." << endl << endl;
	time_t solveStart = time(NULL);

	double maxRunTimeSeconds = 30;
	int nonImprovingIterations = 1000;
	Roadmap bestSolution = LNS::run(randomRoadmap, maxRunTimeSeconds, nonImprovingIterations, 0.25);

	time_t solveEnd = time(NULL);

	// Pretty printing ///////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	cout << "Stories:" << endl << endl;

	for (Story story : storyData) {
		cout << story.toString() << endl;
	}

	cout << endl;

	cout << "Sprints:" << endl << endl;

	for (Sprint sprint : sprintData) {
		cout << sprint.toString() << endl;
	}

	cout << endl;
	cout << "----------------------------------------------------------------" << endl;
	cout << endl;

	cout << bestSolution.printSprintRoadmap() << endl;

	cout << endl;
	cout << "----------------------------------------------------------------" << endl;
	cout << endl;

	cout << "Roadmap value: " << bestSolution.calculateValue() << endl << endl;

	cout << "Is feasible: " << boolalpha << bestSolution.isFeasible() << endl;
	cout << "(" << bestSolution.numberOfSprintCapacitiesViolated() << " sprint capacities exceeded)" << endl;
	cout << "(" << bestSolution.numberOfStoryDependenciesViolated() << " story dependencies unassigned)" << endl;

	cout << endl;
	cout << "Solution found in " << difftime(solveEnd, solveStart) << " seconds" << endl;
}