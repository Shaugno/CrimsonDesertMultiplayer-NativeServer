// @category CrimsonDesert.Port3663
import ghidra.app.script.GhidraScript;
import ghidra.base.project.GhidraProject;

public class R2CopySource extends GhidraScript {
    public void run() throws Exception {
        var source = GhidraProject.openProject(getScriptArgs()[0], getScriptArgs()[1], false);
        try {
            var file = source.getProjectData().getFile("/old/CrimsonDesert.exe");
            if (file == null) throw new IllegalStateException("Missing analyzed source program");
            var folder = state.getProject().getProjectData().getRootFolder();
            if (folder.getFile("CrimsonDesert.exe") != null) throw new IllegalStateException("Source already copied");
            file.copyTo(folder, monitor);
        } finally { source.close(); }
        println("R2 analyzed source copied for VT; no destination markup transferred");
    }
}
