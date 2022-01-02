# API-Project-2019/2020
Theme: A sort of remake of the famous Ed text editor with undo and redo, with the following commands:

-  ind1, ind2c: change the text to the lines between ind1 and ind2, there will be ind2-ind1+1 lines plus a line with a single dot
-  ind1, ind2d: deletes the lines between ind1 and ind2 (extremes included)
-  ind1, ind2p: print the lines between ind1 and ind2
-  nu: undo the last n commands (c or d)
-  nr: redo the last n commands undone with undo
-  q: ends
