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
				if (i == 0)
					dependenciesString += "Story " + to_string(this->dependencies[i]);
				else
					dependenciesString += ", Story " + to_string(this->dependencies[i]);
			}

			return dependenciesString;
		}
		else
			return "None";
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
	vector<Sprint> sprints;

	map<Story, Sprint> storyToSprint;
	map<Sprint, vector<Story>> sprintToStories;

	Roadmap() {};

	Roadmap(vector<Story> stories, vector<Sprint> sprints) {
		this->stories = stories;
		this->sprints = sprints;
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
			if (storyToSprint.find(dependee) == storyToSprint.end())
				// The dependee isn't assigned to a sprint
				return false;

			Sprint dependeeAssignedSprint = storyToSprint[dependee];
			
			// The dependee is assigned to the product backlog
			if (dependeeAssignedSprint.sprintNumber == -1)
				return false;
			
			// The story is assigned to an earlier sprint than its dependee
			if (sprint.sprintNumber <= dependeeAssignedSprint.sprintNumber)
				return false;
		}

		// The sprint doesn't get overloaded and the story's dependencies are satisfied
		return true;
	}

	int storyPointsAssignedToSprint(Sprint sprint) {
		vector<Story> sprintStories = sprintToStories[sprint];

		// Sum of the story points of all the stories assigned to the sprint
		int assignedStoryPoints = 0;

		for (Story story : sprintStories)
			assignedStoryPoints += story.storyPoints;

		return assignedStoryPoints;
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

			// Don't add value from stories assigned to the product backlog
			if (sprint.sprintNumber != -1)
				totalValue += story.businessValue * sprint.sprintBonus;
		}

		return totalValue;
	}

	bool sprintCapacitiesSatisifed() {
		for (pair<Sprint, vector<Story>> pair : sprintToStories) {
			Sprint sprint = pair.first;

			// Don't check the capacity of the product backlog
			if (sprint.sprintNumber != -1) {
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

			// Don't check stories assigned to the product backlog
			if (assignedSprint.sprintNumber != -1) {
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

// Represents moving a story from/to a sprint
class Move {
public:
	Story story;
	Sprint sprint;

	Move() {};

	Move(Story story, Sprint sprint) {
		this->story = story;
		this->sprint = sprint;
	}

	bool operator == (const Move& other) const {
		return this->story == other.story && this->sprint == other.sprint;
	}

	bool operator < (const Move& other) const {
		if (this->story < other.story)
			return true;
		else if (this->story == other.story && this->sprint < other.sprint)
			return true;
		else
			return false;
	}
};

// A partly-destroyed solution, a list of the stories that were removed, and the moves that removed them
class DestroyedSolution {
public:
	Roadmap roadmap;
	vector<Story> removedStories;
	vector<Move> moves;

	DestroyedSolution() {};

	DestroyedSolution(Roadmap roadmap, vector<Story> removedStories, vector<Move> moves) {
		this->roadmap = roadmap;
		this->removedStories = removedStories;
		this->moves = moves;
	}
};

// A repaired solution and the list of moves that repaired it
class RepairedSolution {
public:
	Roadmap roadmap;
	vector<Move> moves;

	RepairedSolution() {};

	RepairedSolution(Roadmap roadmap, vector<Move> moves) {
		this->roadmap = roadmap;
		this->moves = moves;
	}
};

// A Tabu list holding the banned moves
class TabuList {
public:
	map<Move, int> tabuList;
	int tenure;

	TabuList(int tenure) {
		this->tenure = tenure;
	}

	void add(Move move, int currentIteration) {
		// Store the iteration when the move became tabu
		tabuList[move] = currentIteration;
	}

	bool isTabu(Move move, int currentIteration) {
		map<Move, int>::iterator movePositionIterator = tabuList.find(move);

		// Check if the move is in the map
		if (movePositionIterator != tabuList.end()) {
			// The move is in the map, check if its tenure is over
			if (currentIteration - tenure > tabuList[move]) {
				// The move's tenure has expired so it's not tabu - remove it from the map and return
				tabuList.erase(movePositionIterator);
				return false;
			}
			else {
				// The move is tabu
				return true;
			}
		}
		else {
			// It's not in the map so it's not tabu
			return false;
		}
	}
};

// Implements the Large Neighbourhood search algorithm
class LNS {
public:
	LNS() {};

	/*
	// Adapted from:
	// GeeksforGeeks. (2018). Breadth First Search or BFS for a Graph - GeeksforGeeks. [online] Available at: https://www.geeksforgeeks.org/breadth-first-search-or-bfs-for-a-graph/ [Accessed 9 Dec. 2018].
	*/
	// Adds stories to the list of removed stories by traversing the dependency graph in breadth-first order
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

			// Only add the story if it's not already in the list of removed stories
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

	// Removes the given number of stories and its dependencies
	static DestroyedSolution radialRuin(Roadmap completeSolution, int numberOfStoriesToRemove) {
		// The list of the stories removed from the solution
		vector<Story> removedStories;
		vector<Move> moves;

		while (removedStories.size() < numberOfStoriesToRemove) {
			Story randomStory;

			// Keep trying to find a random story that hasn't already been added to the list
			do {
				randomStory = completeSolution.stories[randomInt(0, completeSolution.stories.size() - 1)];
			} while (find(removedStories.begin(), removedStories.end(), randomStory) != removedStories.end());

			// Add the story's dependencies to the list
			removedStories = traverseDependenciesBF(randomStory, completeSolution, removedStories, numberOfStoriesToRemove);
		}

		// Remove the stories from their sprints
		for (Story removedStory : removedStories) {
			Sprint removedStorySprint = completeSolution.storyToSprint[removedStory];
			completeSolution.removeStoryFromSprint(removedStory, removedStorySprint);

			// Add to the list of moves (which will be checked against the tabu list later)
			moves.push_back(Move(removedStory, removedStorySprint));
		}

		return DestroyedSolution(completeSolution, removedStories, moves);
	}

	// Randomly selects stories to remove
	static DestroyedSolution randomRuin(Roadmap completeSolution, int numberOfStoriesToRemove) {
		// The list of the stories removed from the solution
		vector<Story> removedStories;
		vector<Move> moves;

		// Keep removing stories until the required number have been removed
		while (removedStories.size() < numberOfStoriesToRemove) {
			Story randomStory;

			// Keep trying to find a random story that hasn't already been added to the list
			do {
				randomStory = completeSolution.stories[randomInt(0, completeSolution.stories.size() - 1)];
			} while (find(removedStories.begin(), removedStories.end(), randomStory) != removedStories.end());
			Sprint randomStorySprint = completeSolution.storyToSprint[randomStory];

			removedStories.push_back(randomStory);
			completeSolution.removeStoryFromSprint(randomStory, randomStorySprint);

			// Add to the list of moves (which will be checked against the tabu list later)
			moves.push_back(Move(randomStory, randomStorySprint));
		}

		return DestroyedSolution(completeSolution, removedStories, moves);
	}

	// Adds the list of stories to the earliest sprint where they will fit & have their dependencies fulfilled
	static RepairedSolution greedyInsertStories(vector<Story> storiesToInsert, Roadmap roadmap) {
		// The list of moves that repaired the destroyed solution
		vector<Move> moves;

		while (storiesToInsert.size() > 0) {
			Story story = storiesToInsert[0];

			// Greedily re-insert the story into a sprint
			for (Sprint sprint : roadmap.sprints) {
				if (sprint.sprintNumber == -1 || roadmap.validInsert(story, sprint)) {
					roadmap.addStoryToSprint(story, sprint);
					storiesToInsert.erase(remove(storiesToInsert.begin(), storiesToInsert.end(), story), storiesToInsert.end());

					// Add to the list of moves (which will be checked against the tabu list later)
					moves.push_back(Move(story, sprint));

					// Break out of traversing the sprints and move to the next story
					break;
				}
			}
		}

		return RepairedSolution(roadmap, moves);
	}

	// Repair a partly destroyed solution to a complete solution
	static RepairedSolution repair(DestroyedSolution destroyedSolution) {
		sort(destroyedSolution.removedStories.begin(), destroyedSolution.removedStories.end(), StoryGreedySorting());
		return greedyInsertStories(destroyedSolution.removedStories, destroyedSolution.roadmap);
	}

	// Returns whether the temporary solution should become the new current solution
	static bool accept(RepairedSolution repairedSolution, int repairedSolutionValue, int currentSolutionValue, double temperature, int currentIteration, TabuList *tabuList) {
		double delta = repairedSolutionValue - currentSolutionValue;

		// Always accept an improving solution (part of the simulated annealing acceptance and tabu aspiration criteria)
		if (delta >= 0)
			return true;

		// Check if any of the moves made during the repair are tabu
		for (Move move : repairedSolution.moves) {
			// Don't accept the repaired solution if one of the moves is tabu
			if (tabuList->isTabu(move, currentIteration))
				return false;
		}
		
		// Accept non-improving moves with probability related to the annealing temperature
		if (exp(-1 * delta / temperature) > randomDouble(0, 1))
			return true;

		// Didn't meet any of the expected criteria above, just reject the new solution
		return false;
	}

	static Roadmap run(Roadmap currentSolution, int maxIterations, TabuList *tabuList) {
		// TODO
		// - Dynamically set the number of elements to destroy and the Tabu tenure
		//		- if the previous n iterations didn't improve, increase by 1
		//		- cap the maximum

		// The best solution visited so far
		Roadmap bestSolution = currentSolution;
		int bestSolutionValue = bestSolution.calculateValue();
		int currentSolutionValue = bestSolutionValue;

		// Simulated annealing parameters

		double temperature = DBL_MAX;
		double coolingRate = 0.9;
		//double cooledTemperature = 1e-10;

		// Destroy and repair parameters
		
		int ruinMode = 0; // 0 = radial, 1 = random
		double degreeOfDestruction = 0.25;
		int numberOfStoriesToRemove = max(1.0, round(degreeOfDestruction * currentSolution.stories.size()));

		for (int currentIteration = 0; currentIteration < maxIterations; ++currentIteration) {
			// Output to see convergence to optimal over time
			//cout << bestSolution.calculateValue() << endl;

			DestroyedSolution destroyedSolution;
			RepairedSolution repairedSolution;
			
			if (ruinMode == 0) {
				destroyedSolution = radialRuin(currentSolution, numberOfStoriesToRemove);
				repairedSolution = repair(destroyedSolution);
			}
			else if (ruinMode == 1) {
				destroyedSolution = randomRuin(currentSolution, numberOfStoriesToRemove);
				repairedSolution = repair(destroyedSolution);
			}

			ruinMode = (ruinMode + 1) % 2;

			int repairedSolutionValue = repairedSolution.roadmap.calculateValue();

			if (accept(repairedSolution, repairedSolutionValue, currentSolutionValue, temperature, currentIteration, tabuList)) {
				currentSolution = repairedSolution.roadmap;
				currentSolutionValue = repairedSolutionValue;

				// Add the accepted moves to the tabu list:
				// - moves made in the destroyed solution represent moving story A out of sprint B
				// - adding move 'story A -> sprint B' prevents undoing the move
				for (Move move : destroyedSolution.moves)
					tabuList->add(move, currentIteration);

				if (currentSolutionValue > bestSolutionValue && currentSolution.isFeasible()) { // Maximisation
					bestSolution = currentSolution;
					bestSolutionValue = currentSolutionValue;
				}
			}

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

	int maxIterations = 10000;

	// Tabu search parameters

	double tenureRatio = 0.001;
	int tabuTenure = max(1.0, maxIterations * tenureRatio);
	TabuList tabuList(tabuTenure);

	vector<Story> shuffledStories = storyData;
	random_shuffle(shuffledStories.begin(), shuffledStories.end());

	RepairedSolution initialSolution = LNS::greedyInsertStories(shuffledStories, Roadmap(storyData, sprintData));
	
	// Add the intial assignments to the tabu list at iteration 0
	for (Move move : initialSolution.moves)
		tabuList.add(move, 0);

	Roadmap bestSolution = LNS::run(initialSolution.roadmap, maxIterations, &tabuList);

	// Greedily assign any unassigned stories, if possible
	if (!bestSolution.sprintToStories.empty()) {
		Sprint unassignedSprint = sprintData[sprintData.size() - 1];
		vector<Story> unassignedStories = bestSolution.sprintToStories[unassignedSprint];

		// If there are stories assigned to the backlog, try to assign them greedily
		if (!unassignedStories.empty()) {
			// Remove the stories from the 'backlog' sprint
			for (Story unassignedStory : unassignedStories) {
				bestSolution.removeStoryFromSprint(unassignedStory, unassignedSprint);
			}

			sort(unassignedStories.begin(), unassignedStories.end(), StoryGreedySorting());
			bestSolution = LNS::greedyInsertStories(unassignedStories, bestSolution).roadmap;
		}
	}

	auto t_solveEnd = chrono::high_resolution_clock::now();

	// Output to see convergence to optimal over time
	//cout << bestSolution.calculateValue() << endl;

	cout << endl << "LNS" << endl;
	cout << "Stories: " << storyData.size() << ", sprints: " << sprintData.size() - 1 << endl;
	cout << "Solved in " << chrono::duration<double, std::milli>(t_solveEnd - t_initialStart).count() << " ms" << endl;
	cout << "Total weighted business value: " << bestSolution.calculateValue() << endl;
	cout << "----------------------------------------" << endl;

	// Pretty print solution /////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	//cout << endl << "Initial solution -----------------------------------------------" << endl << endl;
	//cout << initialSolution.printSprintRoadmap();

	//cout << endl << "Best solution --------------------------------------------------" << endl << endl;
	//cout << bestSolution.printSprintRoadmap();

	//////////////////////////////////////////////////////////////////////////

	return 0;
}