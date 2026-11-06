# Double Auction Simulator

Double Auction Simulator is a trading game for one player against a market full of bots. Each day you get a secret number: what a crate of mangoes is worth to you, or what it cost you. Sellers step up one at a time and name a price, and every buyer answers at the same moment: take it, counter, or pass. The seller takes the best answer or asks again lower, up to three rounds. You buy on odd days and sell on even days, and whoever has the most profit after seven days wins.

I used raylib for the graphics: the market stage, the characters, and their speech bubbles. The bots each have a personality (stubborn, fair, or nervous) that decides how they bid, counter, and lower their prices, so every trader feels different. For the end of each day, I used insertion sort to line up buyers' values from high to low and sellers' costs from low to high; where the two lists cross is the day's fair price, the same idea as supply meeting demand. Insertion sort also ranks the week's standings.

Fonts: Baloo 2 and Inter (SIL Open Font License, in `fonts/`).
