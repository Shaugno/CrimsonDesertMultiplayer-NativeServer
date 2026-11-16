// @category CrimsonDesert.Port3663
import ghidra.app.script.GhidraScript;
import ghidra.feature.vt.api.main.VTSession;
import java.io.PrintWriter;

public class R2ExportMatches extends GhidraScript {
    public void run() throws Exception {
        var args = getScriptArgs();
        var file = state.getProject().getProjectData().getFile(args[0]);
        if (file == null) throw new IllegalStateException("Missing VT session " + args[0]);
        // As in stock AutoVersionTrackingScript: opening a VT session needs
        // program locks, which the enclosing script transaction would hold.
        end(true);
        var session = (VTSession) file.getDomainObject(this, true, false, monitor);
        try (var out = new PrintWriter(args[1], "UTF-8")) {
            out.println("source\tdestination\ttype\tstatus\tmethod\tsimilarity\tconfidence");
            for (var set : session.getMatchSets()) {
                for (var match : set.getMatches()) {
                    monitor.checkCancelled();
                    var a = match.getAssociation();
                    out.printf("%s\t%s\t%s\t%s\t%s\t%s\t%s%n", match.getSourceAddress(),
                        match.getDestinationAddress(), a.getType(), a.getStatus(),
                        set.getProgramCorrelatorInfo().getName(), match.getSimilarityScore(),
                        match.getConfidenceScore());
                }
            }
        } finally { session.release(this); }
        println("R2 VT export completed");
    }
}
