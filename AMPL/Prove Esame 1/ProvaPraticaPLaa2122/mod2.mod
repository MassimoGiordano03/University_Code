set ESERCIZI;
set GESTI;

param Contributo{GESTI, ESERCIZI};
param MinJoule{GESTI};
param MaxTempo{ESERCIZI};
param PercEsercizi{ESERCIZI};
param Intensita{ESERCIZI};

var tempo_min{ESERCIZI} >= 10;

minimize tempo_allenamento:
	sum{e in ESERCIZI} tempo_min[e];

subject to joule_minimi {g in GESTI}:
	sum{e in ESERCIZI} tempo_min[e]*Contributo[g,e] >= MinJoule[g];
subject to perc_min {e in ESERCIZI}:
	tempo_min[e] >= PercEsercizi[e] * (sum{k in ESERCIZI}tempo_min[k]);
subject to max_intensita:
	sum{e in ESERCIZI}Intensita[e]*tempo_min[e] <=
	16*sum{e in ESERCIZI}tempo_min[e];
subject to min_intensita:	
	15*sum{e in ESERCIZI}tempo_min[e] <= 
	sum{e in ESERCIZI}Intensita[e]*tempo_min[e];
subject to max_tempo {e in ESERCIZI}:
	tempo_min[e] <= MaxTempo[e];