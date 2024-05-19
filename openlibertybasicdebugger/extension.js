// The module 'vscode' contains the VS Code extensibility API
// Import the module and reference it with the alias vscode in your code below
const vscode = require('vscode');

// This method is called when your extension is activated
// Your extension is activated the very first time the command is executed

/**
 * @param {vscode.ExtensionContext} context
 */
function activate(context) 
{
	// Use the console to output diagnostic information (console.log) and errors (console.error)
	// This line of code will only be executed once when your extension is activated
	console.log('Congratulations, your extension "openlibertybasicdebugger" is now active!');

	// The command has been defined in the package.json file
	// Now provide the implementation of the command with  registerCommand
	// The commandId parameter must match the command field in package.json
	context.subscriptions.push(
		vscode.commands.registerCommand('openlibertybasicdebugger.helloWorld', 
		function () 
		{
			// The code you place here will be executed every time your command is executed

			// Display a message box to the user
			vscode.window.showInformationMessage('Hello World from OpenLibertyBasicDebugger!');
		}),
	
		vscode.commands.registerCommand('openlibertybasicdebugger.getProgramName', 
		config => 
		{
			return vscode.window.showInputBox({
				placeHolder: "Please enter the name of a libreBasic file in the workspace folder",
				value: "test.bas"
				});
		}),
		vscode.commands.registerCommand('openlibertybasicdebugger.runEditorContents', 
			(resource) => 
			{
				let targetResource = resource;
				if (!targetResource && vscode.window.activeTextEditor) 
				{
					targetResource = vscode.window.activeTextEditor.document.uri;
				}
				if (targetResource) 
				{
					vscode.debug.startDebugging(undefined, 
						{
							type: 'OpenLibertyBasicDebugger',
							name: 'Run File',
							request: 'launch',
							program: targetResource.fsPath
						},
						{ noDebug: true });
				}
			}),
		vscode.commands.registerCommand('openlibertybasicdebugger.debugEditorContents', 
			(resource) => 
			{
				let targetResource = resource;
				if (!targetResource && vscode.window.activeTextEditor) 
				{
					targetResource = vscode.window.activeTextEditor.document.uri;
				}
				if (targetResource) 
				{
					vscode.debug.startDebugging(undefined, 
						{
							type: 'OpenLibertyBasicDebugger',
							name: 'Debug File',
							request: 'launch',
							program: targetResource.fsPath,
							stopOnEntry: true
						});
				}
			}),
	);

}

// This method is called when your extension is deactivated
function deactivate() 
{
	
}

module.exports = 
{
	activate,
	deactivate
}
