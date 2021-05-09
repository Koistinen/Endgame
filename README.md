Endgame
=======

Compute chess endings with a small number of men completely.

Goals:
In reasonable time compute tables which allows some questions about the endgame to be answered quickly.
For a given position, what is an optimal move and what would result be with optimal play?
Balance initial computation time, storage and answer time.

n is number of pieces including kings and pawns.
Steps:
* Make computation simple and about O(n*S), storage S=2*(64^n) bytes per class (Say KQvK) and answer time pretty quick.
* improve
