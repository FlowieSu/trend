const double summaryReverseResistance =
        blackRedExecuted ? blackRedResult.resistanceKOhm
                         : redBlackResult.resistanceKOhm;

const double summaryReverseCapacitance =
        blackRedExecuted ? blackRedResult.capacitanceNf
                         : redBlackResult.capacitanceNf;

emit testCompleted(item.name,
                   position,
                   item.standard.text,
                   redBlackResult.resistanceKOhm,
                   summaryReverseResistance,
                   redBlackResult.capacitanceNf,
                   summaryReverseCapacitance,
                   pass,
                   message);
