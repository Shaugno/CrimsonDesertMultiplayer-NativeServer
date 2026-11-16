// @category CrimsonDesert.Port3663
// Configure the stock AutoVT task through its documented headless option seam.
import ghidra.app.script.GhidraScript;
import ghidra.feature.vt.gui.util.VTOptionDefines;
import ghidra.features.base.values.GhidraValuesMap;
import java.nio.file.Files;
import java.nio.file.Path;
public class R2VTOptions extends GhidraScript {
    public void run() throws Exception {
        // Official Ghidra example enables every required standard pass and
        // records default thresholds. Change only the optional duplicate pass.
        runScript("SetAutoVersionTrackingOptionsScript.java");
        var options=(GhidraValuesMap)state.getEnvironmentVar("autoVTOptionsMap");
        if (options==null) throw new Exception("Official AutoVT options were not installed");
        options.setBoolean(VTOptionDefines.RUN_DUPE_FUNCTION_OPTION_TEXT,false);
        for (String key : new String[]{VTOptionDefines.CREATE_IMPLIED_MATCHES_OPTION_TEXT,
            VTOptionDefines.RUN_EXACT_SYMBOL_OPTION_TEXT,VTOptionDefines.RUN_EXACT_DATA_OPTION_TEXT,
            VTOptionDefines.RUN_EXACT_FUNCTION_BYTES_OPTION_TEXT,VTOptionDefines.RUN_EXACT_FUNCTION_INST_OPTION_TEXT,
            VTOptionDefines.RUN_REF_CORRELATORS_OPTION_TEXT,VTOptionDefines.APPLY_IMPLIED_MATCHES_OPTION_TEXT}) {
            if (!options.getBoolean(key)) throw new Exception("Required standard VT pass disabled: "+key);
        }
        Files.writeString(Path.of(getScriptArgs()[0]),
            "Stock AutoVT through official SetAutoVersionTrackingOptionsScript.java defaults\n"+
            "Exact symbols/data/function bytes/instructions/mnemonics=true\n"+
            "Data/function/combined reference correlators=true\n"+
            "Create/apply implied matches=true; minimum votes=2; maximum conflicts=0\n"+
            "Function min length=10; data min length=5; symbol min length=3\n"+
            "Reference minimum score=0.95; minimum confidence=10.0\n"+
            "Optional Duplicate Function Correlator=false (only default changed)\n");
        println("R2 standard VT options installed; optional duplicate pass off; all required passes on");
    }
}
