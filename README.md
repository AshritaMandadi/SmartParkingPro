# 🚗 Smart Parking Management System

A menu-driven **Smart Parking System** implemented in C using core **Data Structures** such as:

- Min Heap  
- Circular Queue  
- Singly Linked List  

This project efficiently manages parking slots, waiting queues, billing, and parking history.

---

## 🔧 Features

✔ Automatic slot allocation using **Min Heap**  
✔ Waiting queue using **Circular Queue**  
✔ Parking history stored using **Linked List**  
✔ Monthly pass system (no charges)  
✔ Real-time entry & exit tracking  
✔ Fee calculation based on duration  
✔ Emergency mode (clears parking instantly)  
✔ Revenue tracking  

---

## 📂 File Structure

| File | Description |
|------|-------------|
| `ds.c` | Complete source code |
| `ds` | Compiled executable |

---

## ⚙️ Data Structures Used

### 1️⃣ Min Heap  
Used to always allocate the **smallest available parking slot** efficiently.

### 2️⃣ Circular Queue  
Handles vehicles waiting when all slots are occupied.

### 3️⃣ Linked List  
Stores complete **parking history** with entry & exit time.

---

## 🛠 Compilation

```bash
gcc ds.c -o ds
▶️ Run
bash
Copy code
./ds
📋 Menu Options
mathematica
Copy code
1  - Vehicle Entry
2  - Vehicle Exit
3  - View History
4  - Slot Map
5  - Search Car
6  - Total Revenue
7  - Parked Cars
8  - Waiting Queue
9  - Add Monthly Pass
10 - Emergency Mode
11 - Free Slots
12 - Exit
💰 Fee Policy
₹50 per hour

Rounded up to next hour

Monthly pass users pay ₹0

🚨 Emergency Mode
Clears all parked vehicles

Resets waiting queue

History & revenue remain unchanged

📊 Sample Output
yaml
Copy code
Car 5 parked at Slot 2
Entry : 2025-01-09 10:12:33
Exit  : 2025-01-09 11:30:10
Duration: 1 hr 17 min
Fee: Rs 100
🧠 Learning Outcomes
Practical implementation of Heap

Circular Queue operations

Linked List traversal

Time calculation using time.h

Real-world system simulation

👩‍💻 Author
Ashrita Mandadi
(Data Structures Mini Project)

🚀 Future Enhancements
File storage system

GUI version

Online parking booking

Admin dashboard

Database integration
