_**CS246E Final Design Document**_

![Video Preview](./media/vm-test.gif)

**Introduction**

For the CS246E final project, I chose to create the vim-like text editor: VM. My VM Text Editor uses C++20 and the Ncurses library. The project aims to replicate and extend the functionalities of the built-in Vim editor in most machine terminals. Specifically focusing on implementing both Insert and Command modes, as well as some more advanced features, syntax highlighting and visual mode. Following object-oriented design principles and the Model-View-Controller (MVC) architecture, the project ensures maintainability, scalability, and modularity. This report provides a comprehensive overview of the project's final design, implementation details, encountered challenges, and reflections on the development process.

**Overview**

_Project Structure_

The VM Text Editor is architected following the MVC design pattern, which ensures a clear separation of concerns across different components. The Model layer manages the core data structures, including the text buffer and file operations, thereby handling all data-related logic and maintaining the editor's state consistently. The Model layer contains classes like the Editor and FileManager. The View layer is responsible for the display components, utilizing the Ncurses library to render text, manage the status bar, and provide real-time feedback to user interactions. That View layer contains classes like Display and SyntaxHighlighter to display text, handle changes to text, and display colour for .cc and.h files. Finally, the Controller layer processes user commands, manages different modes (Insert, Command, Visual), and coordinates interactions between the Model and View. This layer contains classes like CommandParser to handle logic for various commands.

_Core_ _Functionalities_

The VM Text Editor handles a broad range of functionalities and commands designed to emulate and/or enhance the user experience provided by Vim. It supports Insert, Command, and Visual modes, enabling users to enter and manipulate text efficiently. Insert mode allows for direct text entry and modification, while Command mode facilitates text manipulation and navigation through a comprehensive set of commands. Visual mode provides a better UI/UX because of the visual highlighting on the selected text and even supports advanced text selection and manipulation capabilities, supporting character-wise, line-wise, and block-wise selections.

A wide array of Vim-like commands have been implemented. The editor supports Insert, Command, and Visual modes, with Visual mode facilitating text highlighting by character, line, and block. All specified commands, including those accessible via colon (:) commands and search functionalities (/ and ?), have been integrated with support for numeric multipliers. For example, text manipulation (yank, delete, change), navigation (h, j, k, l), and file operations (e.g., :w, :q, :wq). These commands support numeric multipliers, enabling users to execute actions multiple times efficiently. The user interface effectively utilizes the entire terminal window, featuring a status bar that displays the current mode or filename on the left and cursor position on the right. Line numbering is implemented, and visual indicators such as tilde (~) symbols denote empty lines beyond the document's end.The editor maintains stability and ensures that unrecognized key combinations do not cause crashes, instead gracefully ignoring such inputs. Additionally, the editor features an unbounded undo system. Syntax highlighting for C++ files enhances code readability by distinguishing keywords, literals, identifiers, comments, and preprocessor directives. The user interface is designed to dynamically occupy the entire terminal window, ensuring an immersive editing experience. A responsive status bar at the bottom displays the current mode or filename, along with row and column information, providing essential feedback to the user.

**Updated UML**

The final design of the VM Text Editor has a few changes from its initial architecture from the first UML. The changes are mainly to accommodate additional features and improve modularity, mostly in the architecture of the Editor class, acting as the Controller, which manages modes, commands, and coordinates interactions between the Model and View. The FileManager class handles all file I/O operations, including loading, saving, and verifying file existence, thus adhering to the Single Responsibility Principle.

The Display class represents the View, managing all aspects of rendering the text buffer, status bar, and user interface elements using Ncurses. This ensures that the editor's visual representation is consistently updated based on the Model's state. The CommandParser class interprets user inputs, including those with numeric multipliers, and instantiates corresponding ICommand objects, decoupling input handling from command execution. The ICommand interface defines a standardized contract for all command classes, facilitating the Command Pattern and enabling seamless command management.

Concrete command classes, such as InsertCommand, DeleteCommand, and UndoCommand, implement the ICommand interface, encapsulating specific actions and adhering to the Single Responsibility Principle. The SyntaxHighlighter class implements syntax highlighting rules for C++ files, distinguishing various code elements through color coding, while the MacroRecorder class manages the recording and playback of macros, leveraging the Command Pattern to store and execute sequences of commands. The StatusBar class within the Display component shows the current mode, filename, cursor position, and provides feedback messages, enhancing user interaction and experience.

**Changes from Initial UML**

Several enhancements have been incorporated into the final UML design. The command hierarchy has been expanded with additional command classes to better encapsulate specific functionalities, ensuring each command adheres to the Single Responsibility Principle. The MacroRecorder class was removed because I did not have time to implement and handle this feature. Additionally, the UML now includes classes responsible for managing Visual mode functionalities, such as selection handling and highlighting mechanisms, which were not part of the initial design. Also for SyntaxHighlighting I created a couple more classes, namely Lexer and Token, to “tokenize” and parse the text to provide the correct syntax highlighting.

**Design**

The VM Text Editor's architecture is meticulously crafted to adhere to the SOLID principles, ensuring a robust, maintainable, and extensible codebase.

Each class in the VM Text Editor is designed with a singular focus, ensuring that they have only one reason to change. The FileManager class, for instance, exclusively handles file operations such as loading, saving, and checking file existence. This encapsulation reduces coupling with other components and simplifies maintenance. Similarly, each command class, such as InsertCommand and DeleteCommand, is responsible solely for a specific text manipulation action. This design ensures that any future modifications to a command do not inadvertently impact other parts of the system, enhancing maintainability and scalability.

The design facilitates the extension of functionalities without modifying existing code. The ICommand interface acts as a contract for all command classes, allowing new commands to be added by simply implementing the interface. This makes it easier to extend new commands and minimizes the risk of introducing bugs into existing functionalities when extending the application. Additionally, this improves code reusability.

All subclasses of ICommand can be substituted wherever ICommand is expected, ensuring flexibility and promoting the use of polymorphism. For example, the Editor class interacts with ICommand objects uniformly, regardless of their concrete implementations. This allows for seamless integration of new commands without requiring changes to the high-level components, thereby adhering to the Liskov Substitution Principle.

Interfaces are designed to be client-specific and minimal. The ICommand interface contains only essential methods (execute, undo, getType), preventing implementing classes from handling unnecessary methods, following the interface segregation principle .

The Dependency Inversion Principle is shown throughout the project when high-level modules depend on abstractions rather than concrete implementations. For example, the Editor class relies on the ICommand interface instead of concrete command classes, allowing it to execute a variety of commands without needing to know their internal implementations. This decouples high-level modules from low-level modules, enhancing flexibility and testability. By depending on abstractions, the system becomes more resilient to changes and easier to extend or modify.

**Design Patterns in Use**

_Command Pattern_

The Command Pattern is central to the VM Text Editor's architecture, encapsulating all actions as objects. The ICommand interface defines the execute and undo methods, establishing a standard for all command classes. Concrete command classes, such as InsertCommand, DeleteCommand, and UndoCommand, implement this interface, providing specific implementations for each action. This pattern not only streamlines command management but also enhances the editor's extensibility, allowing new commands to be integrated effortlessly. A stack-based mechanism maintains a history of executed commands, facilitating the undo functionality by enabling users to revert actions in the reverse order of execution.

_Strategy Pattern_

The Strategy Pattern is applied to manage different modes, encapsulating mode-specific behaviors. The Mode enumeration defines distinct modes (Command, Insert, Visual), each representing a unique set of user interactions and behaviors. The Editor class delegates mode-specific behaviors to corresponding handlers based on the current mode. For instance, in Insert mode, keystrokes are interpreted as text input, whereas in Command mode, they are parsed as commands. This pattern ensures that changes to one mode do not affect others, promoting modularity and ease of maintenance.

**Specific Design Solutions**

Mode Management

The Editor class handles modes with an enumeration and assigns behaviors according to the active mode. This division enables each mode to process user inputs effectively without merging their functionalities. For instance, in Insert mode, keystrokes are sent directly to the text buffer for input, while in Command mode, the inputs are understood as commands for text editing or navigation. This distinct separation guarantees that behaviors specific to each mode are contained within their designated handlers, thereby preserving a structured and orderly architecture.

**Command Execution Framework**

A CommandParser class was implemented to be responsible for interpreting user inputs, which may include numeric multipliers, and for creating the appropriate ICommand objects. This separation of input processing from command execution fosters both extensibility and maintainability. By centralizing the command parsing process, the system can readily integrate new commands or adjust current ones without affecting the overall structure. Additionally, although the input is retrieved from the Editor class. CommandParser handling and executing the commands agrees with the Single Responsibility Principle.

**Syntax Highlighting**

The SyntaxHighlighter class is responsible for processing the text buffer and applying color codes in accordance with established syntax rules. It primarily focuses on fundamental highlighting, like differentiating between keywords, literals, comments, and preprocessor directives. However, the architecture is designed to facilitate straightforward extensions to accommodate more intricate situations, such as contextual keywords and precise identification of mismatched braces, brackets, and parentheses. This modular design guarantees that enhancements to syntax highlighting can be implemented without requiring substantial modifications to the core system, thus aligning with the Open/Closed Principle.

**Extra Credit Features: Visual Mode**

Visual Mode was implemented as a bonus feature to provide users with enhanced text selection and manipulation capabilities. This mode supports three types of selections: character-wise (v), line-wise (V), and block-wise (Ctrl+V). Character-wise selection allows users to highlight individual characters, line-wise selection enables the selection of entire lines, and block-wise selection facilitates rectangular text selection, which is particularly useful for column-based editing. In Visual Mode, users can perform operations such as yank (y), delete (d), and change (c) on the selected text segment. The mode can be toggled dynamically using the corresponding key combinations (v, V, Ctrl+V), providing intuitive control over text manipulation.

Implementing Visual Mode presented several significant challenges, primarily centered around accurately managing different types of text selections and ensuring responsive visual feedback. One major challenge was handling the diverse selection types—character-wise, line-wise, and block-wise—each requiring distinct logic for tracking and rendering. Ensuring that the selection boundaries were correctly identified and maintained during cursor movements necessitated a robust state management system. To address this, the VisualModeHandler class was designed with separate state variables (selStartY, selStartX, selEndY, selEndX) and a selType enumeration to distinctly manage each selection type. This separation of concerns allowed for precise tracking of selection boundaries irrespective of the selection mode.

Another significant challenge was providing real-time visual feedback without compromising the editor's performance. Rendering selections, especially block-wise selections, required efficient handling to prevent screen flickering and lag during rapid cursor movements. The solution involved optimizing the rendering loop to minimize unnecessary redraws. By leveraging Ncurses' capabilities, the implementation ensured that only the relevant portions of the screen were updated when the selection changed. Additionally, conditional checks were implemented to determine whether a character fell within the current selection, applying the appropriate attributes (A_REVERSE for highlighting) only when necessary. This selective rendering approach maintained the editor's responsiveness, providing a smooth and intuitive user experience.

Furthermore, integrating Visual Mode within the existing MVC architecture posed challenges in maintaining a clear separation of concerns. Since Visual Mode can be toggled dynamically using commands like v, V and ctrl-v, just like vim, ensuring that the VisualModeHandler integrated seamlessly with both the Model (text buffer) and the View (display components) required careful design decisions. This was achieved by encapsulating all selection-related logic within the VisualModeHandler class, allowing the Controller (Editor class) to delegate selection responsibilities without intertwining with other components. This modular approach not only preserved the integrity of the MVC structure but also facilitated easier maintenance and potential future enhancements.

**Final Question**

While the project adhered to the MVC architecture, certain areas could benefit from further decoupling to enhance flexibility and maintainability. For instance, ensuring that the Controller interacts with the Model and View through well-defined interfaces would minimize dependencies and promote easier modifications. Implementing Observer Patterns to facilitate communication between the Model and View—where the View subscribes to updates from the Model—would enhance responsiveness and reduce tight coupling. Additionally, leveraging Dependency Injection (DI) to manage dependencies between classes would improve testability and allow for more flexible component swapping or extension.

**Conclusion**

The development of the VM Text Editor was a highly rewarding endeavor that underscored the critical importance of robust object-oriented design principles and their pivotal role in crafting maintainable and scalable software. By adhering to the Model-View-Controller (MVC) architecture, the project achieved a harmonious balance of high cohesion within individual components and low coupling between them. This architectural strategy ensured that each module possessed a clearly defined responsibility while facilitating flexible interactions among them. Consequently, the editor not only became easier to understand, debug, and extend but also demonstrated resilience to future modifications and enhancements.

Throughout the project, the diligent application of SOLID principles and strategic design patterns, such as the Command and Strategy Patterns, further reinforced the system's robustness and adaptability. Implementing complex features like Visual Mode presented substantial challenges, particularly in managing diverse selection types and ensuring responsive visual feedback. These challenges were effectively addressed through sophisticated state management and optimized rendering logic, resulting in a user-friendly and efficient text selection experience. Overall, the VM Text Editor stands as a testament to the effectiveness of object-oriented design, resulting in a powerful and versatile text editing tool that meets current requirements while remaining open to future enhancements.
