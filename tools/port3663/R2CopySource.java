// @category CrimsonDesert.Port3663
import ghidra.app.script.GhidraScript;
import ghidra.base.project.GhidraProject;
import ghidra.framework.main.AppInfo;
import ghidra.program.model.listing.Program;

public class R2CopySource extends GhidraScript {
    public void run() throws Exception {
        var destination = state.getProject();
        var folder = destination.getProjectData().getRootFolder();
        var existing = folder.getFile("CrimsonDesert.exe");
        if (existing != null) {
            var program = (Program)existing.getDomainObject(this, false, false, monitor);
            try {
                if (!program.getExecutableSHA256().equalsIgnoreCase("57DA440D72F4DB974F25FEF047CF84C4DADD999A88CB2A3C5AF4C9BD67FDE1E7") ||
                    program.getListing().getNumInstructions() < 18000000)
                    throw new IllegalStateException("Existing source is not the full analyzed reference");
            } finally { program.release(this); }
            AppInfo.setActiveProject(destination);
            println("R2 reusing verified whole-image source copy; destination active project restored");
            return;
        }
        var source = GhidraProject.openProject(getScriptArgs()[0], getScriptArgs()[1], false);
        try {
            var file = source.getProjectData().getFile("/old/CrimsonDesert.exe");
            if (file == null) throw new IllegalStateException("Missing analyzed source program");
            file.copyTo(folder, monitor);
        } finally {
            source.close();
            // GhidraProject.openProject/close changes the global active project.
            // Stock AutoVT's value parser uses AppInfo, not only script state.
            AppInfo.setActiveProject(destination);
        }
        println("R2 analyzed source copied for VT; no destination markup transferred");
    }
}
