// @category CrimsonDesert.Port3663
import ghidra.app.script.GhidraScript;
import com.google.security.zynamics.BinExport.BinExport2;
import java.io.FileInputStream;
public class R2CompareExports extends GhidraScript {
    private BinExport2 canonical(BinExport2 data) {
        // The exporter iterates a HashSet for call edges; serialization order
        // changes across JVMs. Preserve every edge, sorting only this multiset.
        var edges = new java.util.ArrayList<>(data.getCallGraph().getEdgeList());
        edges.sort(java.util.Comparator.comparing(Object::toString));
        return data.toBuilder()
            .setMetaInformation(data.getMetaInformation().toBuilder().clearTimestamp())
            .setCallGraph(data.getCallGraph().toBuilder().clearEdge().addAllEdge(edges)).build();
    }
    public void run() throws Exception {
        BinExport2 a, b;
        try (var in = new FileInputStream(getScriptArgs()[0])) { a = BinExport2.parseFrom(in); }
        try (var in = new FileInputStream(getScriptArgs()[1])) { b = BinExport2.parseFrom(in); }
        a = canonical(a);
        b = canonical(b);
        if (!a.equals(b)) {
            java.nio.file.Files.writeString(java.nio.file.Path.of(getScriptArgs()[0]+".txt"), a.toString());
            java.nio.file.Files.writeString(java.nio.file.Path.of(getScriptArgs()[1]+".txt"), b.toString());
            throw new Exception("Stock/fixed graph content differs; normalized protobuf text written");
        }
        println("R2 exporter fixture PASS: equal complete protobuf graphs after clearing timestamp and sorting all call edges; " +
            a.getInstructionCount() + " instructions, " + a.getFlowGraphCount() + " flow graphs");
    }
}
