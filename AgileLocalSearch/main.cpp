#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <algorithm>

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

			outputString += story.toString() + " => " + sprint.toString() + "\n";
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

int main(int argc, char* argv[]) {
	// Stories //////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////

	vector<Story> stories;

	Story story0(0, 10, 5);
	Story story1(1, 8, 4, { story0 });
	Story story2(2, 6, 3, { story1 });
	
	stories.push_back(story0);
	stories.push_back(story1);
	stories.push_back(story2);

	cout << "Stories:" << endl;
	
	for (Story story : stories) {
		cout << story.toString() << endl;
	}

	cout << endl;

	// Epics ////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////

	vector<Epic> epics;
	Epic epic0(0, { story0, story1 });
	Epic epic1(1, { story2 });

	epics.push_back(epic0);
	epics.push_back(epic1);

	cout << "Epics:" << endl;

	for (Epic epic : epics) {
		cout << epic.toString() << endl;
	}

	cout << endl;

	// Sprints //////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////

	vector<Sprint> sprints;

	Sprint sprint0(0, 5, 2);
	Sprint sprint1(1, 5, 1);

	sprints.push_back(sprint0);
	sprints.push_back(sprint1);

	cout << "Sprints:" << endl;

	for (Sprint sprint : sprints) {
		cout << sprint.toString() << endl;
	}

	cout << "-------------------------------------------------------------------" << endl;

	// Roadmap //////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////

	Roadmap roadmap(stories, sprints);
	
	roadmap.addStoryToSprint(story0, sprint0);
	roadmap.addStoryToSprint(story1, sprint1);
	roadmap.addStoryToSprint(story2, sprint1);

	cout << roadmap.printStoryRoadmap() << endl;
	cout << roadmap.printSprintRoadmap() << endl;

	cout << "Roadmap value: " << roadmap.calculateValue() << endl << endl;

	cout << "Is feasible: " << boolalpha << roadmap.isFeasible() << endl;
	cout << "(" << roadmap.numberOfSprintCapacitiesViolated() << " sprint capacities exceeded)" << endl;
	cout << "(" << roadmap.numberOfStoryDependenciesViolated() << " story dependencies unassigned)" << endl;
}