# My-OS-is-here-Microsoft-watchout
" if you want to make an OS from scratch you first re-invent the unsiverse " .
OK , i got it the hard way, in this project our goal is to design -> implement -> test & debug .The Main obj's of any real-world OS  
concepts like : memory management (heap & stack ) ,kernel and user and CPU excution scheduling .

## 📌 Overview

This project is an educational operating system built on top of a Stanford-based OS framework (FOS).  
It focuses on implementing and understanding **core operating system concepts**, including:

- Dynamic memory allocation
- Kernel heap management
- Page fault handling
- CPU scheduling and process management

The goal of this project is not only to build working modules, but to deeply understand how modern operating systems manage **memory, performance, and concurrency**.

---

##  Concepts Covered

- Virtual Memory Management
- Paging & Page Tables
- Dynamic Allocation (Block-based allocator)
- Kernel Heap Design
- Page Fault Handling (Placement Strategy)
- CPU Scheduling Algorithms
- Synchronization (Locks & Critical Sections)
- Shared Memory Management

---

## ⚙️ Project Architecture

The system is structured in layers, where core modules act as the foundation for higher-level components:

<img width="833" height="441" alt="image" src="https://github.com/user-attachments/assets/085716d4-f842-46fb-8d27-0546426cdf34" />

Prerequisites (Core Modules):
- Dynamic Allocator
- Kernel Heap
- Fault Handler (Placement)

Built on top:
- User Heap
- Shared Memory
- CPU Scheduler
- Kernel Protection
- Fault Handler (Replacement)


---

## 🧩 Modules Breakdown

###  Dynamic Allocator
- Allocates small memory blocks efficiently
- Uses **power-of-two block sizes**
- Maintains:
  - `freeBlockLists[]` → free blocks grouped by size
  - `freePagesList` → unused pages
  - `pageBlockInfoArr[]` → metadata per page

---

###  Kernel Heap
- Provides dynamic memory allocation inside the kernel
- Supports:
  - Small allocations → Dynamic Allocator
  - Large allocations → Page Allocator
- Implements custom allocation strategies

---

###  Fault Handler (Placement)
- Handles page faults when memory is not present
- Loads required pages from disk into memory
- Updates the working set of the process

---

###  CPU Scheduler
- Manages process execution
- Optimizes CPU utilization
- Supports scheduling strategies

---

###  Shared Memory
- Enables communication between processes
- Handles mapping of shared pages

---

## 🏗️ Memory Design

The allocator divides memory as follows:

- Memory is split into **pages (4KB)**
- Each page is divided into **fixed-size blocks**
- Each page serves **one block size only**

Example:
Page A → 32-byte blocks
Page B → 64-byte blocks
Page C → 128-byte blocks

* this helps you :
1. Find nearest power-of-two size
2. Search in corresponding free block list
3. If empty → allocate new page and split into blocks
4. If no pages available → fallback or panic

---

## 🔒 Synchronization

All shared structures are protected using:
- Locks
- Critical sections

This ensures correctness in concurrent environments.

---

## ▶️ How to Run

1. Clone the repository:
```bash
git clone <your-repo-link>
cd <project-folder>

```
 # tests(init) :
  run test_dynamic_allocator
  run test_kheap
  run test_fault_handler
