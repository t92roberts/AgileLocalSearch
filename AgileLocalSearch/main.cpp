#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <algorithm>
#include <chrono>
#include <queue>
#include <fstream>
#include <sstream>

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
		return ">> Sprint " + to_string(sprintNumber) +
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

class StoryGreedySorting {
public:
	bool operator()(Story const& a, Story const& b) {
		if (a.dependencies.size() < b.dependencies.size())
			return true;
		else if (a.dependencies.size() == b.dependencies.size() && a.businessValue > b.businessValue)
			return true;
		else if (a.dependencies.size() == b.dependencies.size() && a.businessValue == b.businessValue && a.storyPoints < b.storyPoints)
			return true;
		else
			return false;
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

	Roadmap(vector<Story> stories, vector<Sprint> sprints) {
		this->stories = stories;
		this->sprints = sprints;
	}

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

	bool validInsert(Story story, Sprint sprint) {
		// It's always valid to add a story to the product backlog sprint
		if (sprint.sprintNumber == -1)
			return true;

		// Check if adding the story to the sprint overloads the sprint
		if ((story.storyPoints + storyPointsAssignedToSprint(sprint)) > sprint.sprintCapacity)
			return false;

		// Check that each of the story's dependencies are assigned before the sprint
		for (int dependeeNumber : story.dependencies) {
			Story dependee = stories[dependeeNumber];
			// Only stories that are assigned to a sprint are in the map
			if (storyToSprint.find(dependee) == storyToSprint.end()) {
				// The dependee isn't assigned to a sprint
				return false;
			}

			Sprint dependeeAssignedSprint = storyToSprint[dependee];
			
			if (dependeeAssignedSprint.sprintNumber == -1) {
				// The dependee is assigned to the product backlog
				return false;
			}
			
			if (sprint.sprintNumber <= dependeeAssignedSprint.sprintNumber) {
				// The story is assigned to an earlier sprint than its dependee
				return false;
			}
		}

		// The sprint doesn't get overloaded and the story's dependencies are satisfied
		return true;
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

			if (sprint.sprintNumber != -1) { // Don't check the capacity of the product backlog
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

			if (assignedSprint.sprintNumber != -1) { // Don't check stories assigned to the product backlog
				for (int dependeeNumber : story.dependencies) {
					Story dependee = stories[dependeeNumber];
					Sprint dependeeAssignedSprint = storyToSprint[dependee];

					// Only stories that are assigned to a sprint are in the map
					if (storyToSprint.find(dependee) == storyToSprint.end()) {
						// The dependee isn't assigned to a sprint
						dependenciesUnassigned += 1;
					}
					else if (dependeeAssignedSprint.sprintNumber == -1) {
						// The dependee is assigned to the product backlog
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

			if (sprint.sprintNumber != -1) // Don't add value from stories assigned to the product backlog
				totalValue += story.businessValue * sprint.sprintBonus;
		}

		return totalValue;
	}

	bool sprintCapacitiesSatisifed() {
		for (pair<Sprint, vector<Story>> pair : sprintToStories) {
			Sprint sprint = pair.first;

			if (sprint.sprintNumber != -1) { // Don't check the capacity of the product backlog
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

			if (assignedSprint.sprintNumber != -1) { // Don't check stories assigned to the product backlog
				for (int dependeeNumber : story.dependencies) {
					Story dependee = stories[dependeeNumber];
					Sprint dependeeAssignedSprint = storyToSprint[dependee];

					// Only stories that are assigned to a sprint are in the map
					if (storyToSprint.find(dependee) == storyToSprint.end()) {
						// The dependee isn't assigned to a sprint
						return false;
					} else if (dependeeAssignedSprint.sprintNumber == -1) {
						// The dependee is assigned to the special product backlog
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

			if (sprint.sprintNumber != -1) { // Don't check the capacity of the product backlog
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

			if (assignedSprint.sprintNumber != -1) { // Don't check stories assigned to the product backlog
				for (int dependeeNumber : story.dependencies) {
					Story dependee = stories[dependeeNumber];
					Sprint dependeeAssignedSprint = storyToSprint[dependee];

					// Only stories that are assigned to a sprint are in the map
					if (storyToSprint.find(dependee) == storyToSprint.end()) {
						// The dependee isn't assigned to a sprint
						counter += 1;
					}
					else if (dependeeAssignedSprint.sprintNumber == -1) {
						// The dependee is assigned to the product backlog
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
				outputString += "Product Backlog";
			else
				outputString += sprint.toString();

			vector<Story> sprintStories = sprintToStories[sprint];
			int valueDelivered = 0;
			int storyPointsAssigned = 0;

			if (sprintStories.empty()) {
				outputString += "\n\tNone";
			} else {
				for (Story story : sprintStories) {
					valueDelivered += story.businessValue;
					storyPointsAssigned += story.storyPoints;

					outputString += "\n\t" + story.toString();
				}
			}

			outputString += "\n-- [Value: " + to_string(valueDelivered) 
				+ " (weighted value: " + to_string(valueDelivered * sprint.sprintBonus) + "), "
				+ "story points: " + to_string(storyPointsAssigned) + "]";

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

	/*
	// Adapted from:
	// GeeksforGeeks. (2018). Breadth First Search or BFS for a Graph - GeeksforGeeks. [online] Available at: https://www.geeksforgeeks.org/breadth-first-search-or-bfs-for-a-graph/ [Accessed 9 Dec. 2018].
	*/
	static vector<Story> traverseDependenciesBF(Story story, Roadmap roadmap, vector<Story> removedStories, int numberOfStoriesToRemove) {
		bool *visited = new bool[roadmap.stories.size()];

		for (int i = 0; i < roadmap.stories.size(); ++i) {
			visited[i] = false;
		}

		queue<Story> queue;

		visited[story.storyNumber] = true;
		queue.push(story);

		while (!queue.empty() && removedStories.size() < numberOfStoriesToRemove) {
			story = queue.front();
			queue.pop();

			// Prevent removing the same story multiple times
			if (find(removedStories.begin(), removedStories.end(), story) == removedStories.end())
				removedStories.push_back(story);

			for (int dependeeNumber : story.dependencies) {
				Story dependee = roadmap.stories[dependeeNumber];

				if (!visited[dependee.storyNumber]) {
					visited[dependee.storyNumber] = true;
					queue.push(dependee);
				}
			}
		}

		return removedStories;
	}

	static DestroyedSolution radialRuin(Roadmap completeSolution, int numberOfStoriesToRemove) {
		// A list of the stories removed from the solution
		vector<Story> removedStories;

		while (removedStories.size() < numberOfStoriesToRemove) {
			Story randomStory;

			// Prevent trying to remove the same story multiple times
			do {
				randomStory = completeSolution.stories[randomInt(0, completeSolution.stories.size() - 1)];
			} while (find(removedStories.begin(), removedStories.end(), randomStory) != removedStories.end());

			removedStories = traverseDependenciesBF(randomStory, completeSolution, removedStories, numberOfStoriesToRemove);
		}

		// Remove the stories from their sprints
		for (Story removedStory : removedStories) {
			removedStory = completeSolution.stories[removedStory.storyNumber];
			Sprint removedStorySprint = completeSolution.storyToSprint[removedStory];
			completeSolution.removeStoryFromSprint(removedStory, removedStorySprint);
		}

		return DestroyedSolution(completeSolution, removedStories);
	}

	// Randomly selects stories to remove
	static DestroyedSolution randomRuin(Roadmap completeSolution, int numberOfStoriesToRemove) {
		// TODO
		// - Take a Tabu list as an input (to mitigate cycling)

		// A list of the stories removed from the solution
		vector<Story> removedStories;

		// Keep removing stories until the required number have been removed
		while (removedStories.size() < numberOfStoriesToRemove) {
			Story randomStory;

			// Prevent trying to remove the same story multiple times
			do {
				randomStory = completeSolution.stories[randomInt(0, completeSolution.stories.size() - 1)];
			} while (find(removedStories.begin(), removedStories.end(), randomStory) != removedStories.end());
			Sprint randomStorySprint = completeSolution.storyToSprint[randomStory];

			removedStories.push_back(randomStory);
			completeSolution.removeStoryFromSprint(randomStory, randomStorySprint);
		}

		return DestroyedSolution(completeSolution, removedStories);
	}

	static Roadmap greedyInsertStories(vector<Story> storiesToInsert, Roadmap roadmap) {
		while (storiesToInsert.size () > 0) {
			Story story = storiesToInsert[0];
			// Greedily re-insert it into a sprint
			for (Sprint sprint : roadmap.sprints) {
				if (sprint.sprintNumber == -1) {
					// The loop has passed over every sprint and has reached the 'product backlog' sprint
					roadmap.addStoryToSprint(story, sprint);
					storiesToInsert.erase(remove(storiesToInsert.begin(), storiesToInsert.end(), story), storiesToInsert.end());
					break; // Break out of traversing the sprints and move to the next story
				}
				else if (roadmap.validInsert(story, sprint)) {
					// The story can fit into this sprint
					roadmap.addStoryToSprint(story, sprint);
					storiesToInsert.erase(remove(storiesToInsert.begin(), storiesToInsert.end(), story), storiesToInsert.end());
					break; // Break out of traversing the sprints and move to the next story
				}
			}
		}

		return roadmap;
	}

	// Repair a partly destroyed solution to a complete solution
	static Roadmap repair(DestroyedSolution destroyedSolution) {
		// TODO
		// - Use CPLEX to find the optimal way to repair the partly-destroyed solution?

		sort(destroyedSolution.removedStories.begin(), destroyedSolution.removedStories.end(), StoryGreedySorting());
		return greedyInsertStories(destroyedSolution.removedStories, destroyedSolution.roadmap);
	}

	// Returns whether the temporary solution should become the new current solution
	static bool accept(Roadmap temporarySolution, Roadmap currentSolution, double temperature) {
		double delta = (double) temporarySolution.calculateValue() - currentSolution.calculateValue();

		if (delta > 0) // Accept improving moves
			return true;
		else if (exp(-1 * delta / temperature) > randomDouble(0, 1)) // Accept non-improving moves according to the temperature
			return true;
		else
			return false;
	}

	static Roadmap run(Roadmap currentSolution) {
		// TODO
		// - Dynamically set the number of elements to destroy
		//		- if the previous n iterations didn't improve, increase destruction by 1
		//		- cap the maximum destruction
		
		// The best solution visited so far
		Roadmap bestSolution = currentSolution;

		int ruinMode = 0; // 0 = radial, 1 = random

		double temperature = DBL_MAX;
		double coolingRate = 0.9;
		double cooledTemperature = 1e-10;

		while (temperature > cooledTemperature) {
			// Output to see convergence to optimal over time
			//cout << bestSolution.calculateValue() << endl;

			Roadmap temporarySolution;

			double degreeOfDestruction;
			int numberOfStoriesToRemove;
			
			if (ruinMode == 0) {
				degreeOfDestruction = 0.25;
				numberOfStoriesToRemove = max(1.0, round(degreeOfDestruction * currentSolution.stories.size()));
				temporarySolution = repair(radialRuin(currentSolution, numberOfStoriesToRemove));
			}
			else if (ruinMode == 1) {
				degreeOfDestruction = 0.5;
				numberOfStoriesToRemove = max(1.0, round(degreeOfDestruction * currentSolution.stories.size()));
				temporarySolution = repair(randomRuin(currentSolution, numberOfStoriesToRemove));
			}

			ruinMode = (ruinMode + 1) % 2;

			if (accept(temporarySolution, currentSolution, temperature)) {
				currentSolution = temporarySolution;
			}

			if (temporarySolution.calculateValue() > bestSolution.calculateValue() && temporarySolution.isFeasible()) // Maximisation
				bestSolution = temporarySolution;

			temperature *= coolingRate;
		}

		return bestSolution;
	}
};

vector<string> splitString(const string& s, char delimiter) {
	vector<string> tokens;
	string token;
	istringstream tokenStream(s);

	while (getline(tokenStream, token, delimiter)) {
		tokens.push_back(token);
	}

	return tokens;
}

int main(int argc, char* argv[]) {
	// Seed the random number generator
	srand(time(NULL));

	vector<Story> storyData;
	string storyDataFileName;

	// Holds the data about each sprint
	vector<Sprint> sprintData;
	string sprintDataFileName;

	switch (argc) {
	case 3:
		storyDataFileName = argv[1];
		sprintDataFileName = argv[2];
		break;
	default:
		exit(0);
	}

	// Load story data into objects //////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	string line;
	ifstream storiesFile(storyDataFileName);

	getline(storiesFile, line); // Skip column headers

	vector<pair<int, string>> dependenciesStrings;

	if (storiesFile.is_open()) {
		while (getline(storiesFile, line)) {
			vector<string> splitLine = splitString(line, ',');

			int storyNumber = stoi(splitLine[0]);
			int businessValue = stoi(splitLine[1]);
			int storyPoints = stoi(splitLine[2]);

			storyData.push_back(Story(storyNumber, businessValue, storyPoints));

			if (splitLine.size() == 4) {
				string dependencyString = splitLine[3];

				vector<string> splitDependencies = splitString(dependencyString, ';');

				for (string dependeeString : splitDependencies) {
					storyData[storyNumber].dependencies.push_back(stoi(dependeeString));
				}
			}
		}
	} else {
		cout << "Cannot open story data file" << endl;
		exit(0);
	}

	storiesFile.close();

	// Load sprint data into objects /////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	ifstream sprintsFile(sprintDataFileName);

	getline(sprintsFile, line); // Skip column headers

	if (sprintsFile.is_open()) {
		while (getline(sprintsFile, line)) {
			vector<string> splitLine = splitString(line, ',');

			int sprintNumber = stoi(splitLine[0]);
			int sprintCapacity = stoi(splitLine[1]);
			int sprintBonus = stoi(splitLine[2]);

			sprintData.push_back(Sprint(sprintNumber, sprintCapacity, sprintBonus));
		}

		sprintData.push_back(Sprint(-1, 0, 0)); // A special sprint representing 'unassigned' (i.e. assigned to the product backlog)
	} else {
		cout << "Cannot open sprint data file" << endl;
		exit(0);
	}

	sprintsFile.close();

	// Local search //////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	// TODO
	// - Use CPLEX to find an initial feasible, complete solution?
	
	// A complete, feasible roadmap
	//cout << "Building an initial solution..." << endl;
	auto t_initialStart = chrono::high_resolution_clock::now();

	vector<Story> shuffledStories = storyData;
	random_shuffle(shuffledStories.begin(), shuffledStories.end());

	Roadmap initialSolution = LNS::greedyInsertStories(shuffledStories, Roadmap(storyData, sprintData));

	Roadmap bestSolution = LNS::run(initialSolution);

	auto t_solveEnd = chrono::high_resolution_clock::now();

	//cout << "Initial value: " << initialValue << ", final value: " << finalValue << " (" << nearbyint(increase) << "% improvement)" << endl;
	cout << "Solved in " << chrono::duration<double, std::milli>(t_solveEnd - t_initialStart).count() << " ms" << endl << endl;
	cout << "Stories: " << storyData.size() << ", sprints: " << sprintData.size() - 1 << endl;
	cout << "Total weighted business value: " << bestSolution.calculateValue() << endl;
	cout << "--------------------------------------------------------------------------------------------------------" << endl;

	// Pretty print solution /////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	/*cout << endl << "Initial solution -----------------------------------------------" << endl << endl;
	cout << initialSolution.printSprintRoadmap();

	cout << endl << "Best solution --------------------------------------------------" << endl << endl;
	cout << bestSolution.printSprintRoadmap();*/

	//////////////////////////////////////////////////////////////////////////
}