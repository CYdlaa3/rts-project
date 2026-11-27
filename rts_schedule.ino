struct Task {
const char* name;
int C;
int T;
int prio;
int remaining;
int deadline;
int missed;
};

// Tasks: {name, C, T, priority, remaining, deadline, missed}
Task tasks[4] = {
{"cars",   1,  5, 4, 0, 0, 0},
{"button", 6, 20, 3, 0, 0, 0},
{"lights", 5, 30, 2, 0, 0, 0},
{"report",14, 60, 1, 0, 0, 0}
};

const int N = 4;
#define MAX_H 256

char timeline[MAX_H + 1];

int gcdInt(int a, int b) {
while (b) {
int t = a % b;
a = b;
b = t;
}
return a;
}

int hyperperiod() {
int H = tasks[0].T;

for (int i = 1; i < N; i++) {
H = (H * tasks[i].T) / gcdInt(H, tasks[i].T);
}

return H;
}

void simulate() {
int H = hyperperiod();

if (H > MAX_H) {
Serial.println("Hyperperiod too large!");
return;
}

int nextRelease[N] = {0, 0, 0, 0};
int jobRemaining[N] = {0, 0, 0, 0};
int jobDeadline[N] = {0, 0, 0, 0};
bool active[N] = {false, false, false, false};

for (int i = 0; i < N; i++) {
tasks[i].missed = 0;
}

for (int t = 0; t < H; t++) {

// Release jobs and check deadline misses
for (int i = 0; i < N; i++) {

  if (active[i] && t >= jobDeadline[i]) {
    if (jobRemaining[i] > 0) {
      tasks[i].missed++;
    }
    active[i] = false;
  }

  if (t == nextRelease[i]) {
    if (active[i] && jobRemaining[i] > 0) {
      tasks[i].missed++;
    }

    jobRemaining[i] = tasks[i].C;
    jobDeadline[i] = t + tasks[i].T;
    active[i] = true;

    nextRelease[i] += tasks[i].T;
  }
}

// Select highest priority active task
int selected = -1;

for (int i = 0; i < N; i++) {
  if (active[i] && jobRemaining[i] > 0) {
    if (selected == -1 ||
        tasks[i].prio > tasks[selected].prio) {
      selected = i;
    }
  }
}

if (selected == -1) {
  timeline[t] = '-';  // Idle
} else {
  timeline[t] = 'A' + selected;
  jobRemaining[selected]--;

  if (jobRemaining[selected] == 0) {
    active[selected] = false;
  }
}
}

timeline[H] = '\0';

Serial.println("\n--- Fixed Priority Scheduling ---");
Serial.print("Hyperperiod = ");
Serial.println(H);

Serial.println("Timeline (A=cars, B=button, C=lights, D=report):");
Serial.println(timeline);

Serial.println("\nMissed deadlines:");

for (int i = 0; i < N; i++) {
Serial.print(tasks[i].name);
Serial.print(" : ");
Serial.println(tasks[i].missed);
}
}

void setup() {
Serial.begin(9600);
delay(1000);

simulate();
}

void loop() {
}