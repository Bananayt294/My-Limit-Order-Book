# My-Limit-Order-Book

This Limit Order Book is made by me for a fun project as a student.

It uses AVL trees for Limits for O(1) Lookup and has the orders in a linked list format.
It also uses an Order Pool so that it dosent need to constantly create new objects when adding orders or have to delete objects when removing
Might potentially switch to uint32 instead of pointers to avoid cache misses
