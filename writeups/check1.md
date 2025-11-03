Checkpoint 1 Writeup
====================

My name: Hou Binyang

My stuid: 25113050255

I collaborated with: [list sunetids here]

I would like to thank/reward these classmates for their help: [list sunetids here]

This lab took me about [n] hours to do. I [did/did not] attend the lab session.

I was surprised by or edified to learn that: [describe]

Report from the hands-on component of the lab checkpoint: [include
information from 2.1(4), and report on your experience in 2.2]

Describe Reassembler structure and design. [Describe data structures and
approach taken. Describe alternative designs considered or tested.
Describe benefits and weaknesses of your design compared with
alternatives -- perhaps in terms of simplicity/complexity, risk of
bugs, asymptotic performance, empirical performance, required
implementation time and difficulty, and other factors. Include any
measurements if applicable.]

Implementation Challenges:
[]

It's hard to choose the right data structure to store the unassembled segments.
At first, I used a map to store the segments, with the key being the start index of the segment.
However, this approach made it difficult to efficiently merge overlapping segments.
Secondly, I used a list to store the segments. Again, merging overlapping segments proved to be tricky and inefficient.
Finally, I decided to use a vector of Slot to store the unassembled bytes. Each Slot contains a character(byte), a boolean(whether this slot is occupied), and a int(index of the slot). The buffer size is same as the capacity of the Reassembler. Making it easy to find and merge overlapping segments, which improved both the simplicity and performance of the implementation.


Remaining Bugs:
[]

- If applicable: I received help from a former student in this class,
  another expert, or a chatbot or other AI system (e.g. ChatGPT,
  Gemini, Claude, etc.), with the following questions or prompts:
  [please list questions/prompts]

- Optional: I had unexpected difficulty with: [describe]

- Optional: I think you could make this lab better by: [describe]

- Optional: I'm not sure about: [describe]
