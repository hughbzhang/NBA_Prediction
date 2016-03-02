SportsPredictor
===============

This is my attempt to use machine learning to predict who is going to win the next sports game.



6/29

It seems that this project will be significantly harder than initial appearances. Testing the 2013 American League baseball moneyline from oddshark indicated that their bets are very close to perfect accuracy.

Their bets had a score of 6.0184 when grouped in blocks of 10 bets each to reduce variance. A randomized test for if their predictions were 100% on produced a score of 5.360. In addition, since their odds are slightly inflated to produce a profit for themselves, when docked by 1%, the score jumps to 5.9886 showing that their efforts are about 90% of the way to perfect predictions.

Blind Predictor (.5) = 7.7
Oddshark (Raw) = 6.018425
Oddshark (Adjusted -.01)  = 5.988613
Simulated Perfect Guessing = 5.360194

Oddshark winner predictor accuracy = 0.5763689

Although 'perfect prediction' only gets around 0.5755455
IM CONFUSED
SO MAYBE THERE IS HOPE.




I have yet to test my own predictor.


SUMMARY.txt
A sport with a lot of games is probably better for these purposes.

Total number of games in season (including playoffs)

Baseball:
Basketball:
Football:
Golf:
Hockey:






Initial algorithm: Elo ranking or topological sort like my SPARC application.
If your prediction of who is going to win differs from odds by a significant margin, go for it.


Add: web scraper for trades/injuries
Reset how much for each season
Home and away statistics
Head to Head statistics
Hot streaks




Saturday June 14

I did the initial coding and stuff. Pretty much found a website with all basketball data. Learned sed and vim search and replace to manipulate it and then ran an elo system on it. Achieved around 65% accuracy (not bad at all). Moneyline bets for NBA are difficult have data on, so maybe either switch to MLB (hard to find game data) or keep searching for NBA moneyline data.
