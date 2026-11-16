// File-only audit of the stock VT task's reported markup failures.
// @category CrimsonDesert.Port3663
import ghidra.app.script.GhidraScript;
import ghidra.feature.vt.api.main.VTSession;
import ghidra.feature.vt.api.main.VTAssociationStatus;
import ghidra.feature.vt.api.main.VTMarkupItemStatus;
import java.io.PrintWriter;
import java.util.HashSet;
import java.nio.file.Files;
import java.nio.file.Path;

public class R2AuditMarkup extends GhidraScript {
    private String cell(Object value) {
        return String.valueOf(value).replace('\t',' ').replace('\r',' ').replace('\n',' ');
    }
    public void run() throws Exception {
        var args=getScriptArgs();
        if (args.length<2 || args.length>3) throw new IllegalArgumentException("session-path output-tsv [core-addresses]");
        var core=new HashSet<String>();
        if (args.length==3) for (var line:Files.readAllLines(Path.of(args[2]))) core.add(line.trim().toLowerCase());
        var file=state.getProject().getProjectData().getFile(args[0]);
        if (file==null) throw new IllegalStateException("Missing VT session "+args[0]);
        end(true); // release the enclosing program lock before opening its VT session
        var session=(VTSession)file.getDomainObject(this,false,false,monitor);
        var seen=new HashSet<String>();
        long accepted=0, failedAssociations=0, failedItems=0, coreInspected=0;
        try (var out=new PrintWriter(args[1],"UTF-8")) {
            out.println("source\tdestination\ttype\tassociation_status\tmarkup_type\tstatus\tdescription");
            for (var set:session.getMatchSets()) {
                for (var match:set.getMatches()) {
                    monitor.checkCancelled();
                    var association=match.getAssociation();
                    var key=association.getSourceAddress()+"/"+association.getDestinationAddress()+"/"+association.getType();
                    if (!seen.add(key)) continue;
                    if (association.getStatus()==VTAssociationStatus.ACCEPTED) ++accepted;
                    boolean flagged=association.getMarkupStatus().hasErrors();
                    boolean coreMatch=core.contains(association.getSourceAddress().toString().toLowerCase());
                    if (!flagged && !coreMatch) continue;
                    if (flagged) ++failedAssociations;
                    if (coreMatch) ++coreInspected;
                    boolean found=false;
                    for (var item:association.getMarkupItems(monitor)) {
                        if (item.getStatus()!=VTMarkupItemStatus.FAILED_APPLY) continue;
                        found=true; ++failedItems;
                        out.printf("%s\t%s\t%s\t%s\t%s\t%s\t%s%n",association.getSourceAddress(),
                            association.getDestinationAddress(),association.getType(),association.getStatus(),
                            item.getMarkupType().getClass().getSimpleName(),item.getStatus(),
                            cell(item.getStatusDescription()));
                    }
                    if (flagged && !found) out.printf("%s\t%s\t%s\t%s\tunknown\tERROR_FLAG_ONLY\t%s%n",
                        association.getSourceAddress(),association.getDestinationAddress(),association.getType(),association.getStatus(),
                        cell(association.getMarkupStatus().getDescription()));
                }
            }
        } finally {session.release(this);}
        println("R2 markup audit complete: accepted unique="+accepted+
            "; flagged associations="+failedAssociations+"; failed items="+failedItems+
            "; core associations inspected="+coreInspected);
    }
}
