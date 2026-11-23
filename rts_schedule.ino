// Real-time scheduling simulation: RM vs EDF (Wokwi / ESP32 / Arduino)
// Tasks: name, C (execution time), T (period), D = T  (all in ms)
// Output goes to the Serial Monitor.

#define N 4

struct Task {
  const char* name;
  int C;          // execution time
  int T;          // period (= deadline)
  int prio;       // RM priority (1 = highest)
  int remaining;  // remaining execution time in current job
  int deadline;   // absolute deadline of current job
  int missed;     // number of missed deadlines
};

// { name, C, T, prio, remaining, deadline, missed }
Task tasks[N] = {
  { "cars",   1,  5,  1, 0, 0, 0 },
  { "button", 6,  20, 2, 0, 0, 0 },
  { "lights", 5,  30, 3, 0, 0, 0 },
  { "report", 14, 60, 4, 0, 0, 0 },
};

#define MAX_H 256
char timeline[MAX_H + 1];

int gcdInt(int a, int b) {
  while (b) { int t = a % b; a = b; b = t; }
  return a;
}

int hyperperiod() {
  int H = tasks[0].T;
  for (int i = 1; i < N; i++) H = H / gcdInt(H, tasks[i].T) * tasks[i].T;
  return H;
}

double utilization() {
  double u = 0;
  for (int i = 0; i < N; i++) u += (double)tasks[i].C / tasks[i].T;
  return u;
}

double rmBound(int n) {
  return n * (pow(2.0, 1.0 / n) - 1);
}

// Response-time analysis (fixed priority)
int responseTime(int idx) {
  int R = tasks[idx].C;
  while (true) {
    int next = tasks[idx].C;
    for (int j = 0; j < N; j++) {
      if (tasks[j].prio < tasks[idx].prio) {
        next += ((R + tasks[j].T - 1) / tasks[j].T) * tasks[j].C;  // ceil(R/T)*C
      }
    }
    if (next == R || next > tasks[idx].T) return next;
    R = next;
  }
}

// One tick = 1 ms. useEDF = false -> RM, true -> EDF
void simulate(bool useEDF, int H) {
  for (int i = 0; i < N; i++) {
    tasks[i].remaining = 0;
    tasks[i].deadline = 0;
    tasks[i].missed = 0;
  }

  for (int time = 0; time < H; time++) {
    // job releases
    for (int i = 0; i < N; i++) {
      if (time % tasks[i].T == 0) {
        if (tasks[i].remaining > 0) tasks[i].missed++;
        tasks[i].remaining = tasks[i].C;
        tasks[i].deadline = time + tasks[i].T;
      }
    }

    // pick the task to run
    int run = -1;
    for (int i = 0; i < N; i++) {
      if (tasks[i].remaining <= 0) continue;
      if (run == -1) { run = i; continue; }
      if (!useEDF) {
        if (tasks[i].prio < tasks[run].prio) run = i;
      } else {
        if (tasks[i].deadline < tasks[run].deadline) run = i;
      }
    }

    if (run == -1) {
      timeline[time] = '-';
    } else {
      tasks[run].remaining--;
      timeline[time] = tasks[run].name[0];
    }
  }
  timeline[H] = '\0';

  // jobs still unfinished at the end of the hyperperiod
  for (int i = 0; i < N; i++) {
    if (tasks[i].remaining > 0) tasks[i].missed++;
  }

  Serial.print(useEDF ? "EDF " : "RM  ");
  Serial.println(timeline);

  int total = 0;
  for (int i = 0; i < N; i++) {
    if (tasks[i].missed > 0) {
      Serial.print("  missed: ");
      Serial.print(tasks[i].name);
      Serial.print(" x");
      Serial.println(tasks[i].missed);
    }
    total += tasks[i].missed;
  }
  if (total == 0) Serial.println("  missed deadlines: none");
}

void setup() {
  Serial.begin(115200);
  delay(500);

  double U = utilization();
  Serial.print("U = ");
  Serial.print(U, 3);
  Serial.print(", RM bound = ");
  Serial.println(rmBound(N), 3);
  Serial.print("EDF schedulable: ");
  Serial.println(U <= 1.0 ? "true" : "false");

  for (int i = 0; i < N; i++) {
    int R = responseTime(i);
    Serial.print(tasks[i].name);
    Serial.print(" R = ");
    Serial.print(R);
    Serial.println(R <= tasks[i].T ? " OK" : " MISS");
  }

  int H = hyperperiod();
  if (H > MAX_H) {
    Serial.println("Hyperperiod too large for the timeline buffer");
    return;
  }
  Serial.print("Hyperperiod = ");
  Serial.print(H);
  Serial.println(" ms");

  simulate(false, H);  // RM
  simulate(true, H);   // EDF
}

void loop() {
  // nothing to do
}
