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
	vector<int> dependees;

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

	string printDependees() {
		if (this->dependees.size() > 0) {
			string dependeesString = "";

			for (int i = 0; i < this->dependees.size(); ++i) {
				if (i == 0)
					dependeesString += "Story " + to_string(this->dependees[i]);
				else
					dependeesString += ", Story " + to_string(this->dependees[i]);
			}

			return dependeesString;
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
			+ " | story points: " + to_string(storyPoints)
			+ " | dependencies: " + printDependencies() + ")";
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
		// Check if adding the story overloads the sprint
		if (story.storyPoints + storyPointsAssignedToSprint(sprint) > sprint.sprintCapacity && sprint.sprintNumber != -1)
			return false;

		// Check that no dependees are assigned earlier/same as the sprint
		for (int dependeeNumber : story.dependees) {
			Story dependee = stories[dependeeNumber];

			auto it = storyToSprint.find(dependee);

			// The dependee is assigned somewhere
			if (it != storyToSprint.end()) {
				Sprint dependeeAssignedSprint = it->second;

				// The dependee is assigned earlier/same as this sprint
				if (dependeeAssignedSprint.sprintNumber <= sprint.sprintNumber && dependeeAssignedSprint.sprintNumber != -1)
					return false;
			}
		}

		// Check that each of the story's dependencies are assigned before the sprint
		for (int dependencyNumber : story.dependencies) {
			Story dependency = stories[dependencyNumber];

			auto it = storyToSprint.find(dependency);

			// Only stories that are assigned to a sprint are in the map
			if (it == storyToSprint.end())
				// The dependee isn't assigned to a sprint
				return false;

			Sprint dependencyAssignedSprint = it->second;

			// The dependee is assigned to the product backlog
			if (dependencyAssignedSprint.sprintNumber == -1)
				return false;

			// The story is assigned to an earlier sprint than its dependee
			if (sprint.sprintNumber <= dependencyAssignedSprint.sprintNumber)
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
class DestroyedRoadmap {
public:
	Roadmap roadmap;
	vector<Story> removedStories;
	vector<Move> moves;

	DestroyedRoadmap() {};

	DestroyedRoadmap(Roadmap roadmap, vector<Story> removedStories, vector<Move> moves) {
		this->roadmap = roadmap;
		this->removedStories = removedStories;
		this->moves = moves;
	}
};

// A repaired solution and the list of moves that repaired it
class RepairedRoadmap {
public:
	Roadmap roadmap;
	vector<Move> moves;

	RepairedRoadmap() {};

	RepairedRoadmap(Roadmap roadmap, vector<Move> moves) {
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
	static DestroyedRoadmap radialRuin(Roadmap completeSolution, int numberOfStoriesToRemove) {
		// The list of the stories removed from the solution
		vector<Story> removedStories;
		vector<Move> moves;

		// Used to randomly select stories without replacement
		vector<Story> stories = completeSolution.stories;

		while (removedStories.size() < numberOfStoriesToRemove) {
			Story randomStory = stories[randomInt(0, stories.size() - 1)];

			// Add the story's dependencies to the list
			removedStories = traverseDependenciesBF(randomStory, completeSolution, removedStories, numberOfStoriesToRemove);

			for (Story removedStory : removedStories) {
				// Remove the stories from their sprints
				Sprint removedStorySprint = completeSolution.storyToSprint[removedStory];
				completeSolution.removeStoryFromSprint(removedStory, removedStorySprint);

				// Remove the story from the list on unremoved stories
				stories.erase(remove(stories.begin(), stories.end(), removedStory), stories.end());

				// Add to the list of moves (which will be checked against the tabu list later)
				moves.push_back(Move(removedStory, removedStorySprint));
			}
		}

		return DestroyedRoadmap(completeSolution, removedStories, moves);
	}

	// Randomly selects stories to remove
	static DestroyedRoadmap randomRuin(Roadmap completeSolution, int numberOfStoriesToRemove) {
		// The list of the stories removed from the solution
		vector<Story> removedStories;
		vector<Move> moves;

		// Used to randomly select stories without replacement
		vector<Story> stories = completeSolution.stories;

		while (removedStories.size() < numberOfStoriesToRemove) {
			Story randomStory = stories[randomInt(0, stories.size() - 1)];
			Sprint randomStorySprint = completeSolution.storyToSprint[randomStory];

			removedStories.push_back(randomStory);
			completeSolution.removeStoryFromSprint(randomStory, randomStorySprint);

			// Remove the story from the list on unremoved stories
			stories.erase(remove(stories.begin(), stories.end(), randomStory), stories.end());

			// Add to the list of moves (which will be checked against the tabu list later)
			moves.push_back(Move(randomStory, randomStorySprint));
		}

		return DestroyedRoadmap(completeSolution, removedStories, moves);
	}

	// Adds the list of stories to the earliest sprint where they will fit & have their dependencies fulfilled
	static RepairedRoadmap greedyInsertStories(vector<Story> storiesToInsert, Roadmap roadmap) {
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

		return RepairedRoadmap(roadmap, moves);
	}

	// Repair a partly destroyed solution to a complete solution
	static RepairedRoadmap repair(DestroyedRoadmap destroyedSolution) {
		sort(destroyedSolution.removedStories.begin(), destroyedSolution.removedStories.end(), StoryGreedySorting());
		return greedyInsertStories(destroyedSolution.removedStories, destroyedSolution.roadmap);
	}

	// Returns whether the temporary solution should become the new current solution
	static bool accept(RepairedRoadmap repairedSolution, int repairedSolutionValue, int currentSolutionValue, double temperature, int currentIteration, TabuList *tabuList) {
		double delta = repairedSolutionValue - currentSolutionValue;

		// Always accept an improving solution (part of the simulated annealing acceptance and tabu aspiration criteria)
		if (delta > 0)
			return true;

		// Check if any of the moves made during the repair are tabu
		for (Move move : repairedSolution.moves) {
			// Don't accept the repaired solution if one of the moves is tabu
			if (tabuList->isTabu(move, currentIteration))
				return false;
		}

		// Accept non-improving moves with probability related to the annealing temperature
		if (exp(delta / temperature) > randomDouble(0, 1))
			return true;

		// Didn't meet any of the expected criteria above, just reject the new solution
		return false;
	}

	static Roadmap randomRoadmap(vector<Story> storyData, vector<Sprint> sprintData) {
		vector<Story> shuffledStories = storyData;
		random_shuffle(shuffledStories.begin(), shuffledStories.end());
		RepairedRoadmap repairedRoadmap = LNS::greedyInsertStories(shuffledStories, Roadmap(storyData, sprintData));
		return repairedRoadmap.roadmap;
	}

	static int maxDifference(vector<int> integers) {
		int minElement = integers[0];
		int maxElement = integers[1];

		// The maximum difference is the difference between the smallest and largest elements.
		// So find the min and max integers in the list
		for (int element : integers) {
			if (element < minElement)
				minElement = element;
			else if (element > maxElement)
				maxElement = element;
		}

		return maxElement - minElement;
	}

	static double calculateInitialTemperature(vector<Story> storyData, vector<Sprint> sprintData) {
		int trials = storyData.size() * sprintData.size();

		vector<int> randomSolutionValues;

		for (int i = 0; i < trials; ++i) {
			randomSolutionValues.push_back(randomRoadmap(storyData, sprintData).calculateValue());
		}

		return maxDifference(randomSolutionValues);
	}

	static Roadmap run(vector<Story> storyData, vector<Sprint> sprintData) {
		// TODO
		// - Dynamically set the number of elements to destroy and the Tabu tenure
		//		- if the previous n iterations didn't improve, increase by 1
		//		- cap the maximum

		int problemSize = storyData.size() * sprintData.size();

		// Tabu parameters ///////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////

		int tabuTenure = problemSize * 0.05;
		TabuList tabuList(tabuTenure);

		// Simulated annealing parameters ////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////

		double startTemperature = calculateInitialTemperature(storyData, sprintData);
		double temperature = startTemperature;
		double coolingRate = 0.9;

		//////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////

		Roadmap currentSolution = randomRoadmap(storyData, sprintData);
		Roadmap bestSolution = currentSolution;

		// Storing the values saves recomputing the same thing at every iteration when comparing with new solutions
		int currentSolutionValue = currentSolution.calculateValue();
		int bestSolutionValue = currentSolutionValue;

		int ruinMode = 0; // 0 = radial, 1 = random
		double degreeOfDestruction = 0.25;
		int numberOfStoriesToRemove = max(1.0, round(degreeOfDestruction * currentSolution.stories.size()));

		int maxIterations = 2 * problemSize;
		int nonImprovingIterations = 0;
		int maxNonImprovingIterations = maxIterations / 10.0; // maximum of 10 random restarts

		for (int currentIteration = 0; currentIteration < maxIterations; ++currentIteration) {
			//cout << currentSolutionValue << "," << bestSolutionValue << endl;

			if (nonImprovingIterations > maxNonImprovingIterations) {
				nonImprovingIterations = 0;
				temperature = startTemperature; // temperature gets reset when a random restart occurs

				currentSolution = randomRoadmap(storyData, sprintData);
				currentSolutionValue = currentSolution.calculateValue();
			}

			DestroyedRoadmap destroyedSolution;

			if (ruinMode == 0) {
				destroyedSolution = radialRuin(currentSolution, numberOfStoriesToRemove);
			}
			else if (ruinMode == 1) {
				destroyedSolution = randomRuin(currentSolution, numberOfStoriesToRemove);
			}

			ruinMode = (ruinMode + 1) % 2; // alternate to the other ruin mode
			
			RepairedRoadmap repairedSolution = repair(destroyedSolution);
			int repairedSolutionValue = repairedSolution.roadmap.calculateValue();

			if (accept(repairedSolution, repairedSolutionValue, currentSolutionValue, temperature, currentIteration, &tabuList) && repairedSolution.roadmap.isFeasible()) {
				currentSolution = repairedSolution.roadmap;
				currentSolutionValue = repairedSolutionValue;

				// Update the tabu list:
				// - moves made in the destroyed solution represent moving story A out of sprint B
				// - adding move 'story A -> sprint B' prevents undoing the move
				for (Move move : destroyedSolution.moves)
					tabuList.add(move, currentIteration);

				if (currentSolutionValue > bestSolutionValue && currentSolution.isFeasible()) {
					bestSolution = currentSolution;
					bestSolutionValue = currentSolutionValue;

					nonImprovingIterations = 0;
				}
				else {
					++nonImprovingIterations;
				}
			}
			else {
				++nonImprovingIterations;
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

		for (Story story : storyData) {
			for (int dependency : story.dependencies) {
				storyData[dependency].dependees.push_back(story.storyNumber);
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

	auto t_initialStart = chrono::high_resolution_clock::now();
	
	Roadmap bestSolution = LNS::run(storyData, sprintData);

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

	// Pretty print solution /////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	//cout << endl << "Initial solution -----------------------------------------------" << endl << endl;
	//cout << initialSolution.printSprintRoadmap();

	//cout << endl << "Best solution --------------------------------------------------" << endl << endl;
	cout << bestSolution.printSprintRoadmap();

	//////////////////////////////////////////////////////////////////////////

	// Output to see convergence to optimal over time
	//cout << bestSolution.calculateValue() << endl;

	// Output to see value vs elapsed time
	//cout << bestSolution.calculateValue() << "," << chrono::duration<double, std::milli>(t_solveEnd - t_initialStart).count() << endl;

	cout << endl << "LNS" << endl;
	cout << "Stories: " << storyData.size() << ", sprints: " << sprintData.size() - 1 << endl;
	cout << "Solved in " << chrono::duration<double, std::milli>(t_solveEnd - t_initialStart).count() << " ms" << endl;
	cout << "Total weighted business value: " << bestSolution.calculateValue() << endl;
	cout << "----------------------------------------" << endl;

	//cout << storyData.size() << "," << sprintData.size() - 1 << "," << bestSolution.calculateValue() << "," << chrono::duration<double, std::milli>(t_solveEnd - t_initialStart).count();

	return 0;
}