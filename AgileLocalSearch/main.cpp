#include <iostream>
#include <time.h>
#include <string>
#include <vector>
#include <algorithm>
#include <map>

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

	bool operator ==(const Story& other) const {
		return this->storyNumber == other.storyNumber;
	}

	bool operator <(const Story& other) const {
		return this->storyNumber < other.storyNumber;
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
	vector<Story> assignedStories;

	Sprint() {};

	Sprint(int num, int cap, int bonus) {
		this->sprintNumber = num;
		this->sprintCapacity = cap;
		this->sprintBonus = bonus;
	}

	void removeStory(Story story) {
		this->assignedStories.erase(remove(assignedStories.begin(), assignedStories.end(), story), assignedStories.end());
	}

	void addStory(Story story) {
		this->assignedStories.push_back(story);
	}

	bool withinCapacity(int storyPoints) {
		return storyPoints <= this->sprintCapacity;
	}

	int calculateValue() {
		int value = 0;

		for (Story story : this->assignedStories) {
			value += story.businessValue * this->sprintBonus;
		}

		return value;
	}

	bool operator==(const Sprint& other) {
		return this->sprintNumber == other.sprintNumber;
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

	Roadmap() {};

	Roadmap(vector<Story> stories, vector<Sprint> sprints) {
		this->stories = stories;
		this->sprints = sprints;
	}

	void addStoryToSprint(int storyNumber, int sprintNumber) {
		Story story = this->stories[storyNumber];
		Sprint *sprint = &this->sprints[sprintNumber];

		sprint->addStory(story);
	}

	void removeStoryFromSprint(int storyNumber, int sprintNumber) {
		Story story = this->stories[storyNumber];
		Sprint *sprint = &this->sprints[sprintNumber];

		sprint->removeStory(story);
	}

	void moveStory(int storyNumber, int fromSprintNumber, int toSprintNumber) {
		Story story = this->stories[storyNumber];
		Sprint *fromSprint = &this->sprints[fromSprintNumber];
		Sprint *toSprint = &this->sprints[toSprintNumber];

		fromSprint->removeStory(story);
		toSprint->addStory(story);
	}

	bool sprintCapacitiesSatisifed() {
		for (Sprint sprint : this->sprints) {
			int assignedStoryPoints = 0;

			// Sum up the story points of all the stories assigned to the sprint
			for (Story story : sprint.assignedStories) {
				assignedStoryPoints += story.storyPoints;
			}

			// Check if the sprint is overloaded
			if (!sprint.withinCapacity(assignedStoryPoints))
				// The story points assigned are not within the sprint's capacity
				return false;
		}

		// All sprints are filled within capacity
		return true;
	}

	bool storyDependenciesSatisfied() {
		map<Story, Sprint> assignments;

		// Build a mapping from story to sprint (because a sprint knows which stories are assigned to it, but a story doesn't know which sprint it's assigned to)
		for (Sprint sprint : this->sprints) {
			for (Story story : sprint.assignedStories) {
				
				assignments[story] = sprint;
			}
		}

		for (map<Story, Sprint>::iterator it = assignments.begin(); it != assignments.end(); ++it) {
			Story story = it->first;
			Sprint sprint = it->second;

			for (Story dependee : story.dependencies) {
				// Only stories that were assigned to a sprint will have been added to the map, but not all stories may have been assigned
				// Check if the dependee was assigned to any sprint (i.e. it's a key in the map)
				if (assignments.find(dependee) == assignments.end()) {
					// The dependee is not in the map, so it was never assigned to a sprint
					return false;
				} else {
					// Check where the story is assigned compared to its dependee
					if (story.storyNumber <= dependee.storyNumber)
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

	int calculateValue() {
		int value = 0;

		for (Sprint sprint : this->sprints) {
			value += sprint.calculateValue();
		}

		return value;
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
	
	roadmap.addStoryToSprint(0, 0);
	roadmap.addStoryToSprint(1, 1);
	roadmap.removeStoryFromSprint(0, 0);

	cout << "Value: " << roadmap.calculateValue() << endl;
	cout << "Capacities feasible: " << boolalpha << roadmap.sprintCapacitiesSatisifed() << endl;
	cout << "All dependees assigned: " << boolalpha << roadmap.storyDependenciesSatisfied() << endl;
	cout << "Is feasible: " << boolalpha << roadmap.isFeasible() << endl;
}