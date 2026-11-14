// ============================================================
//  UniSync - Final version (Phase 3, 100%)
//  Team: NextGen Coders
//
//  A free-time student interaction and interest-group system.
//
//  Features
//    Phase 1-2 : student registration, free time slots, availability
//                search, groups and group membership
//    Phase 3   : group ratings, merge sort (by rating), binary search
//                (rating filter), hashing (unordered_map), a time-slot
//                index for fast search, inheritance + polymorphism
//                (group types), file saving/loading, speed test
//
//  Compile:  g++ -std=c++17 UniSync_Final.cpp -o unisync
//  Run:      ./unisync        (Windows: .\unisync)
// ============================================================

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <chrono>
#include <random>
#include <iomanip>
#include <limits>
#include <cstdlib>
using namespace std;

// ============================================================
//  Section 1: helper functions (input, time, text)
// ============================================================
long long readInt(const string &prompt) {
    long long value;
    while (true) {
        cout << prompt;
        if (cin >> value) {
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            return value;
        }
        if (cin.eof()) { cout << "\nInput ended. Exiting.\n"; exit(0); }
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "  Invalid input. Please enter a number.\n";
    }
}

string readLine(const string &prompt) {
    string text;
    while (true) {
        cout << prompt;
        if (!getline(cin, text)) { cout << "\nInput ended. Exiting.\n"; exit(0); }
        if (!text.empty()) return text;
        cout << "  Input cannot be empty.\n";
    }
}

// "14:30" -> 870 (minutes since midnight). Returns -1 if wrong.
int parseTime(const string &s) {
    size_t colon = s.find(':');
    if (colon == string::npos || s.size() < 4 || s.size() > 5) return -1;
    try {
        int h = stoi(s.substr(0, colon));
        int m = stoi(s.substr(colon + 1));
        if (h < 0 || h > 23 || m < 0 || m > 59) return -1;
        return h * 60 + m;
    } catch (...) {
        return -1;
    }
}

int readTime(const string &prompt) {
    while (true) {
        int t = parseTime(readLine(prompt));
        if (t != -1) return t;
        cout << "  Wrong format. Use HH:MM, for example 14:30.\n";
    }
}

string timeToString(int minutes) {
    int h = minutes / 60, m = minutes % 60;
    return (h < 10 ? "0" : "") + to_string(h) + ":" + (m < 10 ? "0" : "") + to_string(m);
}

const string DAY_NAMES[] = {"", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};

// Replace characters used as separators in our save files.
string clean(string text) {
    for (size_t i = 0; i < text.size(); i++) {
        if (text[i] == '|' || text[i] == ',' || text[i] == ';' || text[i] == ':') text[i] = '-';
    }
    return text;
}

vector<string> split(const string &text, char sep) {
    vector<string> parts;
    string item;
    stringstream ss(text);
    while (getline(ss, item, sep)) parts.push_back(item);
    return parts;
}

string field(const vector<string> &v, size_t i) { return i < v.size() ? v[i] : ""; }

string lower(string s) {
    for (size_t i = 0; i < s.size(); i++) s[i] = (char)tolower((unsigned char)s[i]);
    return s;
}

// ============================================================
//  Section 2: TimeSlot - one free period (day + start + end)
// ============================================================
class TimeSlot {
private:
    int day;    // 1 = Mon ... 6 = Sat
    int start;  // minutes since midnight
    int end;    // minutes since midnight

public:
    TimeSlot(int d = 1, int s = 0, int e = 0) : day(d), start(s), end(e) {}

    int getDay() const { return day; }
    int getStart() const { return start; }
    int getEnd() const { return end; }

    // Same day, and each slot starts before the other ends.  O(1)
    bool overlaps(const TimeSlot &other) const {
        if (day != other.day) return false;
        return start < other.end && other.start < end;
    }

    string toString() const {
        return DAY_NAMES[day] + " " + timeToString(start) + "-" + timeToString(end);
    }
};

// ============================================================
//  Section 3: Student - one person and their free slots
// ============================================================
class Student {
private:
    long long id;
    string name;
    string section;
    vector<string> interests;
    vector<TimeSlot> slots;

public:
    Student(long long i, const string &n, const string &sec)
        : id(i), name(n), section(sec) {}

    long long getId() const { return id; }
    string getName() const { return name; }
    string getSection() const { return section; }
    const vector<string> &getInterests() const { return interests; }
    const vector<TimeSlot> &getSlots() const { return slots; }

    void addInterest(const string &x) { interests.push_back(x); }
    void addSlot(const TimeSlot &t) { slots.push_back(t); }

    // Is any free slot of this student overlapping the wanted time? O(s)
    bool isFreeAt(const TimeSlot &wanted) const {
        for (size_t i = 0; i < slots.size(); i++) {
            if (slots[i].overlaps(wanted)) return true;
        }
        return false;
    }

    void display() const {
        cout << "  ID: " << id << " | " << name << " | Section: " << section << "\n";
        cout << "     Interests: ";
        if (interests.empty()) cout << "none";
        for (size_t i = 0; i < interests.size(); i++)
            cout << interests[i] << (i + 1 < interests.size() ? ", " : "");
        cout << "\n     Free slots: ";
        if (slots.empty()) cout << "none";
        for (size_t i = 0; i < slots.size(); i++)
            cout << slots[i].toString() << (i + 1 < slots.size() ? ", " : "");
        cout << "\n";
    }

    // One line for the save file: id|name|section|interests|slots
    string serialize() const {
        string line = to_string(id) + "|" + name + "|" + section + "|";
        for (size_t i = 0; i < interests.size(); i++)
            line += interests[i] + (i + 1 < interests.size() ? "," : "");
        line += "|";
        for (size_t i = 0; i < slots.size(); i++) {
            line += to_string(slots[i].getDay()) + ":" + to_string(slots[i].getStart()) +
                    ":" + to_string(slots[i].getEnd()) + (i + 1 < slots.size() ? ";" : "");
        }
        return line;
    }
};

// ============================================================
//  Section 4: Group (base class) and its derived classes
//  INHERITANCE + POLYMORPHISM: describe() is virtual, so each
//  group type gives its own message through a Group pointer.
// ============================================================
class Group {
protected:
    string name;
    string category;
    vector<long long> memberIds;             // only IDs are stored
    unordered_map<long long, int> ratings;   // studentId -> stars (1..5)
    int ratingSum;                           // sum of all stars

public:
    Group(const string &n, const string &c) : name(n), category(c), ratingSum(0) {}
    virtual ~Group() {}

    virtual string describe() const = 0;   // pure virtual: makes Group abstract
    virtual string typeName() const = 0;

    string getName() const { return name; }
    const vector<long long> &getMembers() const { return memberIds; }

    bool hasMember(long long studentId) const {          // O(m)
        for (size_t i = 0; i < memberIds.size(); i++)
            if (memberIds[i] == studentId) return true;
        return false;
    }

    bool addMember(long long studentId) {
        if (hasMember(studentId)) return false;
        memberIds.push_back(studentId);
        return true;
    }

    // Only members can rate. Rating again replaces the old rating.
    bool rate(long long studentId, int stars) {
        if (!hasMember(studentId)) return false;
        unordered_map<long long, int>::iterator it = ratings.find(studentId);
        if (it != ratings.end()) ratingSum -= it->second;
        ratings[studentId] = stars;
        ratingSum += stars;
        return true;
    }

    int ratingCount() const { return (int)ratings.size(); }

    // Average is O(1) because we keep a running sum.
    double average() const {
        return ratings.empty() ? 0.0 : (double)ratingSum / (double)ratings.size();
    }

    string ratingText() const {
        if (ratings.empty()) return "no ratings yet";
        string a = to_string(average());
        return a.substr(0, a.find('.') + 2) + "/5 (" + to_string(ratings.size()) + " rating(s))";
    }

    // type|name|members|ratings
    string serialize() const {
        string line = typeName() + "|" + name + "|";
        for (size_t i = 0; i < memberIds.size(); i++)
            line += to_string(memberIds[i]) + (i + 1 < memberIds.size() ? "," : "");
        line += "|";
        size_t k = 0;
        for (unordered_map<long long, int>::const_iterator it = ratings.begin(); it != ratings.end(); ++it, ++k)
            line += to_string(it->first) + ":" + to_string(it->second) + (k + 1 < ratings.size() ? "," : "");
        return line;
    }
};

class SportsGroup : public Group {
public:
    SportsGroup(const string &n) : Group(n, "Sports") {}
    string typeName() const override { return "Sports"; }
    string describe() const override { return "Sports group: meet at the ground or sports complex and play together."; }
};

class StudyGroup : public Group {
public:
    StudyGroup(const string &n) : Group(n, "Study") {}
    string typeName() const override { return "Study"; }
    string describe() const override { return "Study group: meet in the library or a classroom to revise and solve doubts."; }
};

class CodingGroup : public Group {
public:
    CodingGroup(const string &n) : Group(n, "Coding") {}
    string typeName() const override { return "Coding"; }
    string describe() const override { return "Coding group: practise programming, share projects and prepare for contests."; }
};

class DanceGroup : public Group {
public:
    DanceGroup(const string &n) : Group(n, "Dance") {}
    string typeName() const override { return "Dance"; }
    string describe() const override { return "Dance group: practise routines in the auditorium or a free hall."; }
};

class OtherGroup : public Group {
public:
    OtherGroup(const string &n) : Group(n, "Other") {}
    string typeName() const override { return "Other"; }
    string describe() const override { return "General interest group: meet and share a common hobby."; }
};

// Factory: chooses the right derived class from the category text.
unique_ptr<Group> makeGroup(const string &category, const string &name) {
    string c = lower(category);
    if (c == "sports") return unique_ptr<Group>(new SportsGroup(name));
    if (c == "study")  return unique_ptr<Group>(new StudyGroup(name));
    if (c == "coding") return unique_ptr<Group>(new CodingGroup(name));
    if (c == "dance")  return unique_ptr<Group>(new DanceGroup(name));
    return unique_ptr<Group>(new OtherGroup(name));
}

// ============================================================
//  Section 5: StudentManager
//  HASHING: idIndex (id -> position) gives O(1) average lookup.
//  TIME-SLOT INDEX: hourIndex maps (day, hour) -> students who
//  are free in that hour, so a search only checks a few students.
// ============================================================
struct CommonWindow {
    const Student *other;
    int day, from, to;
};

class StudentManager {
private:
    vector<Student> students;
    unordered_map<long long, size_t> idIndex;
    unordered_map<int, unordered_set<long long> > hourIndex;

    // Add one slot to the hour buckets it covers.
    void indexSlot(long long id, const TimeSlot &t) {
        for (int h = t.getStart() / 60; h <= (t.getEnd() - 1) / 60; h++)
            hourIndex[t.getDay() * 24 + h].insert(id);
    }

    // IDs of students having a free slot in any hour touched by 't'.
    unordered_set<long long> candidateIds(const TimeSlot &t) const {
        unordered_set<long long> ids;
        for (int h = t.getStart() / 60; h <= (t.getEnd() - 1) / 60; h++) {
            unordered_map<int, unordered_set<long long> >::const_iterator it = hourIndex.find(t.getDay() * 24 + h);
            if (it != hourIndex.end()) ids.insert(it->second.begin(), it->second.end());
        }
        return ids;
    }

public:
    // Hash lookup: O(1) average
    Student *findById(long long id) {
        unordered_map<long long, size_t>::iterator it = idIndex.find(id);
        return it == idIndex.end() ? nullptr : &students[it->second];
    }
    const Student *find(long long id) const {
        unordered_map<long long, size_t>::const_iterator it = idIndex.find(id);
        return it == idIndex.end() ? nullptr : &students[it->second];
    }

    bool registerStudent(const Student &s) {
        if (idIndex.count(s.getId())) return false;      // duplicate ID
        idIndex[s.getId()] = students.size();
        students.push_back(s);
        for (size_t i = 0; i < s.getSlots().size(); i++) indexSlot(s.getId(), s.getSlots()[i]);
        return true;
    }

    bool addSlot(long long id, const TimeSlot &t) {
        Student *s = findById(id);
        if (s == nullptr) return false;
        s->addSlot(t);
        indexSlot(id, t);
        return true;
    }

    // Phase 2 method: check every student.  O(n * s)
    vector<const Student *> findAvailableLinear(const Student &me, const TimeSlot &wanted) const {
        vector<const Student *> result;
        for (size_t i = 0; i < students.size(); i++) {
            const Student &st = students[i];
            if (st.getId() == me.getId() || st.getSection() == me.getSection()) continue;
            if (st.isFreeAt(wanted)) result.push_back(&st);
        }
        return result;
    }

    // Phase 3 method: check only students from the time-slot index.
    // About O(k + c*s): k hour buckets, c candidate students.
    vector<const Student *> findAvailableIndexed(const Student &me, const TimeSlot &wanted) const {
        vector<const Student *> result;
        unordered_set<long long> ids = candidateIds(wanted);
        for (unordered_set<long long>::const_iterator it = ids.begin(); it != ids.end(); ++it) {
            const Student *st = find(*it);
            if (st == nullptr) continue;
            if (st->getId() == me.getId() || st->getSection() == me.getSection()) continue;
            if (st->isFreeAt(wanted)) result.push_back(st);
        }
        sort(result.begin(), result.end(),
             [](const Student *a, const Student *b) { return a->getId() < b->getId(); });
        return result;
    }

    // Compare my own slots with everyone else's; return exact common windows.
    vector<CommonWindow> commonFreeTime(const Student &me) const {
        vector<CommonWindow> result;
        for (size_t a = 0; a < me.getSlots().size(); a++) {
            const TimeSlot &mine = me.getSlots()[a];
            unordered_set<long long> ids = candidateIds(mine);
            for (unordered_set<long long>::const_iterator it = ids.begin(); it != ids.end(); ++it) {
                const Student *st = find(*it);
                if (st == nullptr) continue;
                if (st->getId() == me.getId() || st->getSection() == me.getSection()) continue;
                for (size_t b = 0; b < st->getSlots().size(); b++) {
                    const TimeSlot &theirs = st->getSlots()[b];
                    if (mine.overlaps(theirs)) {
                        CommonWindow w;
                        w.other = st;
                        w.day = mine.getDay();
                        w.from = max(mine.getStart(), theirs.getStart());
                        w.to = min(mine.getEnd(), theirs.getEnd());
                        result.push_back(w);
                    }
                }
            }
        }
        sort(result.begin(), result.end(), [](const CommonWindow &x, const CommonWindow &y) {
            if (x.day != y.day) return x.day < y.day;
            if (x.from != y.from) return x.from < y.from;
            return x.other->getId() < y.other->getId();
        });
        return result;
    }

    void displayAll() const {
        if (students.empty()) { cout << "  No students registered yet.\n"; return; }
        for (size_t i = 0; i < students.size(); i++) students[i].display();
    }

    int count() const { return (int)students.size(); }
    const vector<Student> &all() const { return students; }
};

// ============================================================
//  Section 6: sorting and searching algorithms for groups
// ============================================================

// Merge two sorted halves a[left..mid] and a[mid+1..right].
// Highest average rating first. Ties keep their old order (stable).
void mergeHalves(vector<Group *> &a, int left, int mid, int right) {
    vector<Group *> temp;
    temp.reserve(right - left + 1);
    int i = left, j = mid + 1;
    while (i <= mid && j <= right) {
        if (a[i]->average() >= a[j]->average()) temp.push_back(a[i++]);
        else temp.push_back(a[j++]);
    }
    while (i <= mid) temp.push_back(a[i++]);
    while (j <= right) temp.push_back(a[j++]);
    for (size_t k = 0; k < temp.size(); k++) a[left + (int)k] = temp[k];
}

// Merge sort: O(n log n) time, O(n) extra space.
void mergeSortGroups(vector<Group *> &a, int left, int right) {
    if (left >= right) return;               // 0 or 1 item: already sorted
    int mid = (left + right) / 2;
    mergeSortGroups(a, left, mid);
    mergeSortGroups(a, mid + 1, right);
    mergeHalves(a, left, mid, right);
}

// Binary search on a list sorted high -> low.
// Returns how many groups have average >= minRating.  O(log n)
int countWithRatingAtLeast(const vector<Group *> &sorted, double minRating) {
    int lo = 0, hi = (int)sorted.size();
    while (lo < hi) {
        int mid = (lo + hi) / 2;
        if (sorted[mid]->average() >= minRating) lo = mid + 1;
        else hi = mid;
    }
    return lo;
}

// ============================================================
//  Section 7: GroupManager
//  HASHING: nameIndex (name -> Group*) gives O(1) average lookup.
// ============================================================
class GroupManager {
private:
    vector<unique_ptr<Group> > groups;
    unordered_map<string, Group *> nameIndex;

public:
    Group *findByName(const string &name) {
        unordered_map<string, Group *>::iterator it = nameIndex.find(name);
        return it == nameIndex.end() ? nullptr : it->second;
    }

    bool createGroup(const string &name, const string &category) {
        if (nameIndex.count(name)) return false;     // duplicate name
        groups.push_back(makeGroup(category, name));
        nameIndex[name] = groups.back().get();
        return true;
    }

    vector<Group *> allGroups() const {
        vector<Group *> v;
        for (size_t i = 0; i < groups.size(); i++) v.push_back(groups[i].get());
        return v;
    }

    int count() const { return (int)groups.size(); }
};

// ============================================================
//  Section 8: saving and loading (file handling)
// ============================================================
const string STUDENT_FILE = "unisync_students.txt";
const string GROUP_FILE = "unisync_groups.txt";

bool saveAll(const StudentManager &sm, const GroupManager &gm) {
    ofstream sf(STUDENT_FILE.c_str());
    ofstream gf(GROUP_FILE.c_str());
    if (!sf || !gf) return false;
    const vector<Student> &st = sm.all();
    for (size_t i = 0; i < st.size(); i++) sf << st[i].serialize() << "\n";
    vector<Group *> gs = gm.allGroups();
    for (size_t i = 0; i < gs.size(); i++) gf << gs[i]->serialize() << "\n";
    return true;
}

void loadAll(StudentManager &sm, GroupManager &gm) {
    int ns = 0, ng = 0;
    ifstream sf(STUDENT_FILE.c_str());
    string line;
    while (sf && getline(sf, line)) {
        vector<string> p = split(line, '|');
        if (p.size() < 3) continue;
        try {
            Student s(stoll(p[0]), p[1], p[2]);
            vector<string> ints = split(field(p, 3), ',');
            for (size_t i = 0; i < ints.size(); i++) s.addInterest(ints[i]);
            vector<string> sl = split(field(p, 4), ';');
            for (size_t i = 0; i < sl.size(); i++) {
                vector<string> q = split(sl[i], ':');
                if (q.size() != 3) continue;
                int d = stoi(q[0]), a = stoi(q[1]), b = stoi(q[2]);
                if (d >= 1 && d <= 6 && a < b) s.addSlot(TimeSlot(d, a, b));
            }
            if (sm.registerStudent(s)) ns++;
        } catch (...) {}
    }
    ifstream gf(GROUP_FILE.c_str());
    while (gf && getline(gf, line)) {
        vector<string> p = split(line, '|');
        if (p.size() < 2) continue;
        try {
            if (!gm.createGroup(p[1], p[0])) continue;
            Group *g = gm.findByName(p[1]);
            vector<string> mem = split(field(p, 2), ',');
            for (size_t i = 0; i < mem.size(); i++) g->addMember(stoll(mem[i]));
            vector<string> rt = split(field(p, 3), ',');
            for (size_t i = 0; i < rt.size(); i++) {
                vector<string> q = split(rt[i], ':');
                if (q.size() == 2) g->rate(stoll(q[0]), stoi(q[1]));
            }
            ng++;
        } catch (...) {}
    }
    if (ns > 0 || ng > 0)
        cout << "Loaded " << ns << " student(s) and " << ng << " group(s) from saved files.\n";
}

// ============================================================
//  Section 9: menu actions (kept outside the classes)
// ============================================================
int readDay() {
    while (true) {
        long long d = readInt("  Day (1=Mon, 2=Tue, 3=Wed, 4=Thu, 5=Fri, 6=Sat): ");
        if (d >= 1 && d <= 6) return (int)d;
        cout << "  Day must be between 1 and 6.\n";
    }
}

TimeSlot readTimeSlot() {
    int day = readDay();
    while (true) {
        int s = readTime("  Start time (HH:MM): ");
        int e = readTime("  End time (HH:MM): ");
        if (s < e) return TimeSlot(day, s, e);
        cout << "  End time must be after start time. Try again.\n";
    }
}

Student *askForStudent(StudentManager &sm, const string &prompt) {
    Student *s = sm.findById(readInt(prompt));
    if (s == nullptr) cout << "  Student not found.\n";
    return s;
}

void registerStudentMenu(StudentManager &sm) {
    cout << "\n--- Register Student ---\n";
    long long id = readInt("  Student ID: ");
    if (sm.findById(id) != nullptr) { cout << "  This ID is already registered.\n"; return; }
    string name = clean(readLine("  Name: "));
    string section = clean(readLine("  Section (e.g. A1): "));
    Student s(id, name, section);
    long long ni = readInt("  How many interests do you want to add? ");
    for (long long i = 0; i < ni; i++) s.addInterest(clean(readLine("   Interest: ")));
    long long ns = readInt("  How many free time slots do you want to add? ");
    for (long long i = 0; i < ns; i++) {
        cout << "  Slot " << (i + 1) << ":\n";
        s.addSlot(readTimeSlot());
    }
    sm.registerStudent(s);
    cout << "  Student registered successfully.\n";
}

void addSlotMenu(StudentManager &sm) {
    cout << "\n--- Add Free Slot ---\n";
    Student *s = askForStudent(sm, "  Student ID: ");
    if (s == nullptr) return;
    sm.addSlot(s->getId(), readTimeSlot());
    cout << "  Slot added.\n";
}

void searchMenu(StudentManager &sm) {
    cout << "\n--- Find Available Students (indexed search) ---\n";
    Student *me = askForStudent(sm, "  Your Student ID: ");
    if (me == nullptr) return;
    cout << "  Enter the time you want to meet:\n";
    TimeSlot wanted = readTimeSlot();
    vector<const Student *> r = sm.findAvailableIndexed(*me, wanted);
    cout << "\n  Students free on " << wanted.toString() << ":\n";
    if (r.empty()) cout << "   No students from other sections are free at that time.\n";
    for (size_t i = 0; i < r.size(); i++)
        cout << "   - " << r[i]->getName() << " (ID " << r[i]->getId() << ", Section " << r[i]->getSection() << ")\n";
}

void commonTimeMenu(StudentManager &sm) {
    cout << "\n--- Common Free Time (uses your own slots) ---\n";
    Student *me = askForStudent(sm, "  Your Student ID: ");
    if (me == nullptr) return;
    vector<CommonWindow> r = sm.commonFreeTime(*me);
    cout << "\n  Common free time for " << me->getName() << ":\n";
    if (r.empty()) cout << "   No common free time with students from other sections.\n";
    for (size_t i = 0; i < r.size(); i++)
        cout << "   - " << DAY_NAMES[r[i].day] << " " << timeToString(r[i].from) << "-" << timeToString(r[i].to)
             << " with " << r[i].other->getName() << " (ID " << r[i].other->getId()
             << ", Section " << r[i].other->getSection() << ")\n";
}

void createGroupMenu(GroupManager &gm) {
    cout << "\n--- Create Group ---\n";
    string name = clean(readLine("  Group name: "));
    string category = readLine("  Category (Sports/Study/Coding/Dance/Other): ");
    if (gm.createGroup(name, category)) cout << "  Group created.\n";
    else cout << "  A group with this name already exists.\n";
}

void joinGroupMenu(StudentManager &sm, GroupManager &gm) {
    cout << "\n--- Join Group ---\n";
    Student *s = askForStudent(sm, "  Your Student ID: ");
    if (s == nullptr) return;
    Group *g = gm.findByName(clean(readLine("  Group name: ")));
    if (g == nullptr) { cout << "  Group not found.\n"; return; }
    if (g->addMember(s->getId())) cout << "  " << s->getName() << " joined " << g->getName() << ".\n";
    else cout << "  You are already a member of this group.\n";
}

void rateGroupMenu(StudentManager &sm, GroupManager &gm) {
    cout << "\n--- Rate Group ---\n";
    Student *s = askForStudent(sm, "  Your Student ID: ");
    if (s == nullptr) return;
    Group *g = gm.findByName(clean(readLine("  Group name: ")));
    if (g == nullptr) { cout << "  Group not found.\n"; return; }
    if (!g->hasMember(s->getId())) { cout << "  Only members of a group can rate it. Join first.\n"; return; }
    long long stars;
    while (true) {
        stars = readInt("  Rating (1 to 5): ");
        if (stars >= 1 && stars <= 5) break;
        cout << "  Rating must be between 1 and 5.\n";
    }
    g->rate(s->getId(), (int)stars);
    cout << "  Thank you! " << g->getName() << " now has " << g->ratingText() << ".\n";
}

void showGroupsSorted(GroupManager &gm) {
    cout << "\n--- Groups sorted by rating (merge sort) ---\n";
    vector<Group *> v = gm.allGroups();
    if (v.empty()) { cout << "  No groups created yet.\n"; return; }
    mergeSortGroups(v, 0, (int)v.size() - 1);
    for (size_t i = 0; i < v.size(); i++)
        cout << "  " << (i + 1) << ". " << v[i]->getName() << " [" << v[i]->typeName() << "] - "
             << v[i]->ratingText() << " - " << v[i]->getMembers().size() << " member(s)\n";
}

void showGroupsAtLeast(GroupManager &gm) {
    cout << "\n--- Groups with a minimum rating (binary search) ---\n";
    vector<Group *> v = gm.allGroups();
    if (v.empty()) { cout << "  No groups created yet.\n"; return; }
    long long minRating;
    while (true) {
        minRating = readInt("  Minimum average rating (1 to 5): ");
        if (minRating >= 1 && minRating <= 5) break;
        cout << "  Enter a number from 1 to 5.\n";
    }
    mergeSortGroups(v, 0, (int)v.size() - 1);                 // list must be sorted first
    int n = countWithRatingAtLeast(v, (double)minRating);     // then binary search
    if (n == 0) cout << "  No group has an average of " << minRating << " or more.\n";
    for (int i = 0; i < n; i++)
        cout << "  " << (i + 1) << ". " << v[i]->getName() << " [" << v[i]->typeName() << "] - "
             << v[i]->ratingText() << "\n";
}

void showGroupMembers(StudentManager &sm, GroupManager &gm) {
    cout << "\n--- Group Members ---\n";
    Group *g = gm.findByName(clean(readLine("  Group name: ")));
    if (g == nullptr) { cout << "  Group not found.\n"; return; }
    cout << "  Members of " << g->getName() << " (" << g->ratingText() << "):\n";
    if (g->getMembers().empty()) cout << "   (no members yet)\n";
    for (size_t i = 0; i < g->getMembers().size(); i++) {
        const Student *s = sm.find(g->getMembers()[i]);
        if (s != nullptr)
            cout << "   - " << s->getName() << " (ID " << s->getId() << ", Section " << s->getSection() << ")\n";
    }
}

// Polymorphism demo: the same call describe() gives a different message per type.
void showGroupTypes(GroupManager &gm) {
    cout << "\n--- Group types (polymorphism) ---\n";
    vector<Group *> v = gm.allGroups();
    if (v.empty()) { cout << "  No groups created yet.\n"; return; }
    for (size_t i = 0; i < v.size(); i++)
        cout << "  " << v[i]->getName() << " -> " << v[i]->describe() << "\n";
}

// Compare the Phase 2 linear search with the Phase 3 indexed search.
void speedTest() {
    cout << "\n--- Speed test: linear search vs indexed search ---\n";
    const int N = 20000, REPEAT = 200, SLOTS = 2;
    StudentManager big;
    mt19937 rng(12345);
    for (int i = 1; i <= N; i++) {
        Student s(i, "S" + to_string(i), "Sec" + to_string(rng() % 10));
        for (int k = 0; k < SLOTS; k++) {
            int day = 1 + (int)(rng() % 6);
            int start = (8 + (int)(rng() % 9)) * 60 + (int)(rng() % 60);
            s.addSlot(TimeSlot(day, start, start + 30 + (int)(rng() % 61)));
        }
        big.registerStudent(s);
    }
    const Student *me = big.find(1);
    TimeSlot wanted(1, 10 * 60, 10 * 60 + 30);
    size_t linearCount = 0, indexedCount = 0;

    auto t1 = chrono::steady_clock::now();
    for (int r = 0; r < REPEAT; r++) linearCount = big.findAvailableLinear(*me, wanted).size();
    auto t2 = chrono::steady_clock::now();
    for (int r = 0; r < REPEAT; r++) indexedCount = big.findAvailableIndexed(*me, wanted).size();
    auto t3 = chrono::steady_clock::now();

    double linearUs = chrono::duration<double, micro>(t2 - t1).count() / REPEAT;
    double indexedUs = chrono::duration<double, micro>(t3 - t2).count() / REPEAT;
    cout << "  Students: " << N << " (" << SLOTS << " free slots each), search: Mon 10:00-10:30\n";
    cout << "  Linear search  : " << (long long)linearUs << " microseconds per search, "
         << linearCount << " result(s)\n";
    cout << "  Indexed search : " << (long long)indexedUs << " microseconds per search, "
         << indexedCount << " result(s)\n";
    cout << "  Same results: " << (linearCount == indexedCount ? "yes" : "NO") << "\n";
    if (indexedUs > 0)
        cout << "  Indexed search is about " << fixed << setprecision(1) << (linearUs / indexedUs)
             << "x faster here (the gain depends on how many students match).\n";
}

void loadSampleData(StudentManager &sm, GroupManager &gm) {
    struct Row { long long id; const char *name; const char *sec; const char *interest; int day, s, e; };
    Row rows[] = {
        {101, "Ashutosh", "A1", "Coding", 1, 10 * 60, 11 * 60},
        {102, "Akash", "B2", "Sports", 1, 10 * 60 + 30, 12 * 60},
        {103, "Meenakshi", "C1", "Dance", 1, 9 * 60, 10 * 60 + 30},
        {104, "Ayush", "A1", "Study", 2, 14 * 60, 15 * 60}};
    for (int i = 0; i < 4; i++) {
        Student s(rows[i].id, rows[i].name, rows[i].sec);
        s.addInterest(rows[i].interest);
        s.addSlot(TimeSlot(rows[i].day, rows[i].s, rows[i].e));
        sm.registerStudent(s);
    }
    gm.createGroup("CodeClub", "Coding");
    gm.createGroup("Cricket Crew", "Sports");
    gm.createGroup("Dance Floor", "Dance");
    gm.createGroup("Library Circle", "Study");
    long long mem[][2] = {{101, 0}, {102, 0}, {103, 2}, {102, 1}, {104, 3}, {101, 3}};
    const char *names[] = {"CodeClub", "Cricket Crew", "Dance Floor", "Library Circle"};
    for (int i = 0; i < 6; i++) gm.findByName(names[mem[i][1]])->addMember(mem[i][0]);
    gm.findByName("CodeClub")->rate(101, 5);
    gm.findByName("CodeClub")->rate(102, 4);
    gm.findByName("Cricket Crew")->rate(102, 3);
    gm.findByName("Dance Floor")->rate(103, 5);
    gm.findByName("Library Circle")->rate(104, 4);
    cout << "  Sample data loaded (4 students, 4 groups with ratings).\n";
    cout << "  Try: option 4 (ID 101, Mon 10:00-11:00) and option 9.\n";
}

void printMenu() {
    cout << "\n=============== UniSync (Final) ===============\n"
         << "  1. Register student\n"
         << "  2. Add free slot to a student\n"
         << "  3. Show all students\n"
         << "  4. Find available students (at a time I choose)\n"
         << "  5. Common free time (uses my own slots)\n"
         << "  6. Create group\n"
         << "  7. Join group\n"
         << "  8. Rate group\n"
         << "  9. Show groups sorted by rating\n"
         << " 10. Show groups with minimum rating\n"
         << " 11. Show group members\n"
         << " 12. Show group types\n"
         << " 13. Speed test (linear vs indexed search)\n"
         << " 14. Load sample data\n"
         << " 15. Save data now\n"
         << "  0. Save and exit\n"
         << "===============================================\n";
}

// ------------------------------------------------------------
//  main(): load data, show the menu, save on exit
// ------------------------------------------------------------
int main() {
    StudentManager sm;
    GroupManager gm;
    loadAll(sm, gm);
    int choice;

    do {
        printMenu();
        choice = (int)readInt("Enter choice: ");
        switch (choice) {
            case 1: registerStudentMenu(sm); break;
            case 2: addSlotMenu(sm); break;
            case 3: cout << "\n--- All Students ---\n"; sm.displayAll(); break;
            case 4: searchMenu(sm); break;
            case 5: commonTimeMenu(sm); break;
            case 6: createGroupMenu(gm); break;
            case 7: joinGroupMenu(sm, gm); break;
            case 8: rateGroupMenu(sm, gm); break;
            case 9: showGroupsSorted(gm); break;
            case 10: showGroupsAtLeast(gm); break;
            case 11: showGroupMembers(sm, gm); break;
            case 12: showGroupTypes(gm); break;
            case 13: speedTest(); break;
            case 14: loadSampleData(sm, gm); break;
            case 15: cout << (saveAll(sm, gm) ? "  Data saved.\n" : "  Could not save data.\n"); break;
            case 0: break;
            default: cout << "  Invalid choice. Try again.\n";
        }
    } while (choice != 0);

    cout << (saveAll(sm, gm) ? "Data saved. Goodbye!\n" : "Could not save data. Goodbye!\n");
    return 0;
}
