Ιωάννης Καπετανγεώργης
Α.Μ.: 1115201800061

Προγραμματισμός Συστημάτων - Project 2
---------------------------------------

Γενικά σχόλια:
--------------
- Η υλοποίηση του πρώτου part της εργασίας περιέχεται στο directory με όνομα TravelMonitor. Αντίστοιχα, η υλοποίηση του δεύτερου part περιέχεται στο directory με όνομα BashScript.
- Όλα τα αρχεία κώδικα είναι σχολιασμένα πολύ αναλυτικά και εξηγείται η κάθε λειτουργία που γίνεται στο κάθε σημείο.
- Όση μνήμη έχει δεσμευθεί δυναμικά κατά την εκτέλεση του προγράμματος, αποδεσμεύεται πριν το τέλος αυτού με ελεγχόμενο τρόπο και χωρίς να υπάρχουν απώλειες μνήμης η errors. Έχει ελεγχθεί μέσω του valgrind.
- Ο κώδικας έχει χωριστεί σε πολλά αρχεία (.c και .h) ανάλογα με την λειτουργία που υλοποιούν έτσι ώστε να επιτευχθεί abstraction των δεδομένων αλλά και να είναι πιο κατανοητός και ευανάγνωστος ο συνολικός κώδικας. Επίσης επιτυγχάνεται πλήρης απόκρυψη πληροφορίας. Για κάθε αρχείο κώδικα (.c) (εκτός του αρχείου main.c που περιέχει μόνο την main) υπάρχουν και τα αντίστοιχα header αρχεία, τα οποία περιέχουν τις απαραίτητες δηλώσεις έτσι ώστε να παρέχεται το "interface" σε ένα άλλο αρχείο το οποίο χρησιμοποιεί τις λειτουργίες αυτές.
- Σε όλα τα σημεία που αποθηκεύεται μια συμβολοσειρά, χρησιμοποιώ τον τύπο char * (και όχι κάποιον στατικό πίνακα χαρακτήρων π.χ. char [50]) έτσι ώστε να δεσμεύεται μόνο όσος χώρος χρειάζεται για την αποθήκευση της συμβολοσειράς. Εξαίρεση σε αυτό αποτελούν μόνο κάποιοι προσωρινοί buffers κατά το διάβασμα εντολών του χρήστη και εγγραφών του αρχείου.
- Έχει υλοποιηθεί το απαραίτητο error checking σχετικά με την εισαγωγή των δεδομένων (τόσο από το input αρχείο όσο και από της εντολές του χρήστη). Έχουν ληφθεί υπόψιν αρκετές (σε ορισμένες λειτουργίες και όλες) οι πιθανές περιπτώσεις λάθος εισόδου. Σε κάθε περίπτωση τυπώνεται μήνυμα λάθους.
- Τέλος, έχουν υλοποιηθεί όλα όσα ζητούνται στην εκφώνηση αλλά και αναφέρθηκαν στο piazza.

Compilation και εκτελέσιμο πρόγραμμα:
--------------------------------------
- Στο παραδοτέο directory υπάρχει το Makefile μέσω του οποίου μπορούν να παραχθούν τα εκτελέσιμα προγράμματα. Εκτελώντας την εντολή make παράγεται το εκτελέσιμο πρόγραμμα με όνομα travelMonitor και το εκτελέσιμο με όνομα Monitor.
- Εφαρμόζεται separate compilation, δηλαδή πρώτα παράγονται όλα τα object file και τελικά μέσω αυτών παράγεται το εκτελέσιμο πρόγραμμα.
- Όπως αναφέρεται και στην εκφώνηση, εκτελούμε το travelMonitor, στο όποιο έπειτα από τα κατάλληλα forks και exec calls τα παιδιά "τρέχουν" το εκτελέσιμο Monitor.
- Για την εκτέλεση του προγράμματος πρέπει να δοθούν τα ορίσματα τα οποία περιγράφονται στην εκφώνηση. Διαφορετικά εκτυπώνεται μήνυμα λάθους. Δηλαδή, η εκτέλεση γίνεται ως εξής:
	./travelMonitor –m numMonitors -b bufferSize -s sizeOfBloom-i input_dir
 *(τα ορίσματα μπορούν να δοθούν με οποιαδήποτε σειρά )

Βασικά σημεία της εργασίας:
---------------------------
1) Η παράμετρος bufferSize μπορεί να είναι αρκετά μικρότερη από ότι τα μηνύματα που θέλουμε να στείλουμε. Αρχικά οι μόνοι τύποι μηνυμάτων που στέλνονται μεταξύ travelMonitor και Monitors είναι strings και bloomFilters. Επίσης, για την αποστολή strings χρησιμοποιώ ένα "πρωτόκολο" κατά το οποίο στέλνεται πρώτα το μήκος του string που πρόκειται να σταλθεί έτσι ώστε να γνωρίζει ο παραλήπτης πόσα bytes πρόκειται να διαβάσει. Επομένως για την αποστολή ενός string, χρειάζεται να προηγηθεί πρώτα η αποστολή ενός integer. Για την αποστολή/ανάγνωση μηνυμάτων έχω υλοποιήσει αντίστοιχες συναρτήσεις οι οποίες διαχειρίζονται το "πρόβλημα" του bufferSize και στέλνουν τα μηνύματα αυτά με κατάλληλο τρόπο.
Πιο συγκεκριμένα, για την αποστολή strings έχω υλοποιήσει τις συναρτήσεις: sendMessage και readMessage (κα sendInt, readInt για την αποστολή/ανάγνωση του μήκους του string).
Για την αποστολή ενός string αρχικά στέλνεται το μήκος του string που πρόκειται να σταλθεί μέσω της συνάρτησης sendInt. Σε περίπτωση που το bufferSize είναι μεγαλύτερο από το sizeOf(int), τότε η sendInt απλά στέλνει τον αριθμό με ένα μήνυμα μεγέθους sizeOf(int). Αν το bufferSize είναι μικρότερο από το sizeOf(int), τότε ο αριθμός στέλνεται σε "κομμάτια", δηλαδή με πολλαπλά μηνύματα το πολύ μεγέθους bufferSize. Στην συνέχεια με μια επαναληπτική διαδικασία στέλνουμε το string σε "κομμάτια" όπου το κάθε ένα έχει μέγεθος το πολύ bufferSize. Στην περίπτωση όπου το string που θέλουμε να στείλουμε είναι μεγαλύτερο από bufferSize τότε όλα τα μηνύματα/"κομμάτια" πλην του τελευταίου είναι μεγέθους bufferSize ενώ το τελευταίο ενδεχομένως να είναι λιγότερα από bufferSize. Αντίστοιχα, για το διάβασμα του string αρχικά διαβάζεται το μήκος αυτού του string μέσω της readInt. Στην συνέχεια, εφόσον γνωρίζουμε το μήκος του string, μέσω μιας επαναληπτικής διαδικασίας διαβάζουμε το string σε "κομμάτια" γνωρίζοντας μέσω του μήκους του string το μέγεθος του επόμενου "κομματιού" που πρόκειται να διαβάσουμε. Τελικά ενώνοντας τα κομμάτια αυτά έχουμε το ενιαίο string.
Για την αποστολή/ανάγνωση ενός bloomFilter ακολουθείται παρόμοια διαδικασία από τις συναρτήσεις: sendBloom και readBloom. Η βασική διαφορά είναι ότι δεν χρειάζεται να στέλνουμε το μέγεθος του bloomFilter (όπως κάνουμε με το μήκος του string) καθώς είναι πάντα το ίδιο (bloomSize) και είναι γνωστό τόσο από τον travelMonitor όσο και από τα Monitors. Ένα bloomFilter είναι ένας πίνακας από bytes. Επομένως στέλνουμε το bloomFilter σε "κομμάτια" μεγέθους bufferSize, δηλαδή στέλνουμε κάθε φορά bufferSize θέσεις του πίνακα. Προφανώς το τελευταίο μήνυμα μπορεί να είναι μικρότερου μεγέθους από bufferSize. Αντίστοιχα για την ανάγνωση μέσα σε μια επαναληπτική διαδικασία διαβάζουμε κάθε φορά bufferSize bytes και τα αντιγράφουμε κάθε φορά στην κατάλληλη θέση ενός ενιαίου πίνακα bloomSize μεγέθους έτσι ώστε να "συνθέσουμε" το συνολικό bloomFilter.
Έτσι έχουμε καταφέρει κάθε μήνυμα να είναι το πολύ μεγέθους bufferSize ανεξαρτήτως από τον τύπο και το μέγεθος του μηνύματος που θέλουμε να στείλουμε/διαβάσουμε. Η μόνη εξαίρεση είναι το πρώτο μήνυμα που "ανταλλάζουν" travelMonitor και Monitor και περιέχει το bufferSize. Έπειτα από αυτό το μήνυμα, όλα τα επόμενα μηνύματα στέλνονται μέσω των συναρτήσεων που εξηγούνται παραπάνω και στέλνουν πάντα μηνύματα το πολύ μεγέθους bufferSize.
Οι παραπάνω συναρτήσεις υλοποιούνται στο αρχείο pipeReadWrite.c.
 ΠΑΡΑΤΗΡΗΣΗ: Το bufferSize δεν έχει κάποιο κάτω όριο. Το πρόγραμμα λειτουργεί κανονικά ακόμα και για bufferSize = 1 byte στέλνοντας μόνο μήνυματα ενός byte καθ' όλη την διάρκεια της εκτέλεσης.
2) Σε περίπτωση που ένα Monitor λάβει ένα σήμα USR1 ενώ βρίσκεται την μέση αποστολής δεδομένων, ολοκληρώνει την αποστολή των δεδομένων αυτών και στην συνέχεια κάνει της απαραίτητες λειτουργίες που σχετίζονται με το USR1 signal. Πιο συγκεκριμένα, αφού ολοκληρώσει την αποστολή του τρέχον μηνύματος, βρίσκει τα νέα αρχεία τα οποία έχουν προστεθεί στα directories τα οποία "διαχειρίζεται", διαβάζει τα αρχεία αυτά, εισάγει τις εγγραφές στην "βάση δεδομένων" και τέλος στέλνει τα ενημερωμένα bloomFilters στον travelMonitor. Κάθε φορά που ένα monitor λαμβάνει ένα USR1 signal, ο signal handler που έχει οριστεί θέλει μια flag μεταβλητή σε 1 έτσι ώστε να "δείξει" ότι έλαβε ένα USR1 signal. Ο monitor κάθε φορά που ολοκληρώνει την εκτέλεση μιας λειτουργίας (π.χ. /travelRequest) αλλά και όσο περιμένει κάποια λειτουργία, ελέγχει αυτό το flag και σε περίπτωση που είναι 1, εκτελεί τις λειτουργίες που αναφέρθηκαν.
Αντίστοιχη διαχείριση γίνεται και για τα σήματα SIGQUIT και SIGINT. Χρησιμοποιείται ένα αντίστοιχο flag το οποίο ο signal handler θέτει σε 1 κάθε φορά που λαμβάνει ένα σήμα. Ομοίως, όσο ο monitor "περιμένει" κάποια εντολή από τον travelMonitor (και γενικότερα όσο βρίσκεται σε "αδράνεια") ελέγχει συνεχώς αν το flag αυτό έχει γίνει 1. Αν το flag είναι 1, καλείτε η συνάρτηση writeLogFile η οποία δημιουργεί ένα logFile και γράφει σε αυτό τις πληροφορίες που περιγράφονται στην εκφώνηση. Έπειτα, συνεχίζεται κανονικά η εκτέλεση του monitor.
Κάθε φορά που εκτελείται μια από τις παραπάνω λειτουργίες (διάβασμα νέου αρχείου / γράψιμο logFile) επαναφέρεται το flag σε 0, καθώς έχουμε "διαχειριστεί" κατάλληλα το αντίστοιχο σήμα.
3) Το parent process διαβάζει μόνο από pipes τα οποία περιέχουν δεδομένα "για διάβασμα", δηλαδή σε περιπτώσεις όπου θέλουμε να διαβάσουμε από πολλά παιδιά συγχρόνως (π.χ. όταν όλα τα παιδιά στέλνουν τα bloomFilters τους) ο travelMonitor δεν μπλοκάρει ποτέ περιμένοντας κάποιο παιδί. Αυτό επιτυγχάνεται με την χρήση της poll() (της βιβλιοθήκης <poll.h>). Μέσω της poll, κάθε φορά ελέγχουμε σε ποια pipes υπάρχουν δεδομένα για διάβασμα και διαβάζουμε μόνο από αυτά τα pipes. Έτσι, το parent process δεν περιμένει ποτέ κάποιο αργό παιδί, αντιθέτως διαβάσει κάθε φορά τα δεδομένα των "γρήγορων" παιδιών τα οποία έχουν ήδη σταλεί. Το παραπάνω περιέχεται εσωτερικά μιας επαναληπτικής διαδικασίας. Έτσι, σε κάθε επανάληψη διαβάζει ένα μήνυμα από όσα παιδιά έχουν προλάβει να στείλουν. Όταν ένα παιδί στείλει όλα του τα δεδομένα, στέλνει ένα προκαθορισμένο μήνυμα έτσι ώστε να δείξει στον πατέρα ότι έστειλε όλα του τα δεδομένα. Έτσι, όταν ο πατέρας έχει λάβει αυτό το προκαθορισμένο μήνυμα από όλα τα παιδιά, η επαναληπτική διαδικασία ολοκληρώνεται καθώς έχουμε διαβάσει όλα τα δεδομένα όλων των παιδιών.
4) Αντίστοιχα με όσα περιγράφηκαν στο 3), τα Monitor processes δεν μπλοκάρουν περιμένοντας κάποια εντολή από τον χρήστη. Αντιθέτως, μέσω της poll ελέγχουν αν το travelMonitor τους έχει στείλει κάποιο μήνυμα/εντολή. Αν τους έχει στείλει τότε διαβάζουν το μήνυμα, και εκτελούν την αντίστοιχη εντολή. Αν δεν τους έχει στείλει κάποιο μήνυμα/εντολή τότε ελέγχουν τα flags που περιγράφουν παραπάνω, ελέγχουν δηλαδή αν έχουν λάβει κάποιο σήμα (και εκτελούν την αντίστοιχη λειτουργία αν έχουν όντως λάβει κάποιο σήμα). Η διαδικασία αυτή, δηλαδή έλεγχος αν έχει έρθει μήνυμα / έλεγχος αν έχει έρθει σήμα, εκτελείται επαναληπτικά.
5) Όπως περιγράφηκε και στο 1) το πρωτόκολλο επικοινωνίας για μηνύματα συμβολοσειρών είναι ότι πρώτα στέλνεται το μήκος της συμβολοσειράς και στην συνέχεια διαβάζεται. Στα bloomFilters το μέγεθος είναι προκαθορισμένο και γνωστό επομένως δεν χρειάζεται τέτοιο πρωτόκολλο. Επομένως σε κάθε περίπτωση ο παραλήπτης γνωρίζει το μέγεθος του μηνύματος που πρέπει να διαβάσει. Επίσης σε κάθε σημείο του προγράμματος, ο παραλήπτης γνωρίζει αν περιμένει να διαβάσει ένα string ή ένα bloomFilter καθώς οι κλήσεις read/write είναι πάντα συμμετρικές. Επομένως σε κάθε πιθανή περίπτωση ο παραλήπτης ξέρει πως να ερμηνεύσει τα bytes τα οποία διαβάζει.

Επιπλέον παρατηρήσεις:
---------------------------
- Για κάθε ένα monitor το father process δημιουργεί 2 named pipes για την μεταξύ τους επικοινωνία. Ένα για να γράφει ο πατέρας και να διαβάζει το παιδί και ένα για να γράφει το παιδί και να διαβάζει ο πατέρας.
- Όπως αναφέρθηκε και παραπάνω, το bufferSize μπορεί να πάρει οποιαδήποτε τιμή (ακόμη και 1 byte). Η τιμή αυτή βέβαια, δεν πρέπει να ξεπερνά τα όρια χωρητικότητας των pipes (το οποίο διαφέρει από μηχάνημα σε μηχάνημα).
- Για την ανάγνωση των περιεχομένων ενός directory έχω χρησιμοποιήσει την βιβλιοθήκη dirent.h.
- Για την διαχείριση των signals έχω χρησιμοποιήσει την sigaction, ορίζοντας τους κατάλληλους handlers για κάθε signal.
- Επίσης, η διαχείρηση των σημάτων εξηγείται αναλυτικά στις παραγράφους TravelMonitor και Monitor. Παρακαλώ να δοθεί προσοχή στην ΣΗΜΑΝΤΙΚΗ ΠΑΡΑΤΗΡΗΣΗ της παραγράφου TravelMonitor έτσι ώστε να γίνει κατανοητό το πως/πότε διαχειρίζεται τα signals το travelMonitor
- Inconsistent εγγραφές στα αρχεία εισόδου απορρίπτονται ακριβώς όπως περιγραφόταν στην εργασία 1.
- Σε περίπτωση όπου ο αριθμός των χωρών/directories (έστω numOfCountries) στο input directory είναι μικρότερος από το numMonitors, τότε ανατίθεται μία χώρα στα πρώτα numOfCountries monitors, ενώ τα υπόλοιπα τερματίζουν. Επομένως έχουμε numOfCountries ενεργά monitors και όχι numMonitors.
- Η δομές που χρησιμοποιούνται από τους Monitors για την απάντηση των ερωτημάτων, αλλά και τα bloomFilters είναι τα ίδια με αυτά που είχα υλοποιήσει στην πρώτη εργασία. Πιο συγκεκριμένα οι βασικές δομές που χρησιμοποιεί το κάθε monitor για την απάντηση των ερωτημάτων είναι:
	-Ένα hashTable το οποίο περιέχει τις πληροφορίες του κάθε πολίτη.
	-Μια λίστα από ιούς, όπου κάθε κόμβος της αντιστοιχεί σε έναν ιό και περιέχει: μια vaccinated_skipList, μια not_vaccinated_skipList και ένα bloomFilter (το οποίο στέλνεται στον πατέρα).
	-Επίσης χρησιμοποιούνται και άλλες απλούστερες δομές, για παράδειγμα λίστα από χώρες.

Travel Monitor:
---------------------------
Η διαδικασία που ακολουθεί ο travelMonitor είναι η εξής:
- Αρχικά μέσω της sigaction ορίζει τους κατάλληλους signal handlers για τα σήματα SIGCHLD και SIGINT/SIGQUIT.
- Στην συνέχεια μέσω μιας επαναληπτικής διαδικασίας (τόσων επαναλήψεων όσος και ο αριθμός των monitors):
	- Δημιουργεί 2 pipes για την επικοινωνία με το συγκεκριμένο παιδί
	- Κάνει fork. Έπειτα από το fork η διεργασία παιδί, καλεί την execl με όρισμα το εκτελέσιμο Monitor καθώς και σαν επιπλέον ορίσματα
	  τα ονόματα των 2 pipes που μόλις δημιουργήθηκαν. Η διεργασία πατέρας, ανοίγει τα 2 αυτά pipes και αποθηκεύει τους 2 file descriptors.
	- Αρχικοποιεί τις δομές οι οποίες θα αποθηκεύουν πληροφορίες σχετικά με το παιδί αυτό. Η δομές αυτές είναι: μια λίστα στην οποία θα
	  αποθηκευτούν τα bloomFilters τα οποία θα του στείλει αυτό το παιδί και μια λίστα η οποία θα περιέχει τις χώρες οι οποίες θα ανατεθούν
	  στο παιδί αυτό. Επίσης αποθηκεύει το process id του παιδιού.
- Στέλνει σε κάθε ένα παιδί, το bufferSize και το bloomSize.
- Στην συνέχεια διαβάζει το directory που δόθηκε σαν όρισμα, εντοπίζει τις χώρες/φακέλους που περιέχονται σε αυτό και τις κρατά σε μια ταξινομημένη λίστα.
- Με Round Robin κατανομή, στέλνει στο κάθε παιδί τις χώρες για τις οποίες θα είναι υπεύθυνο. Για κάθε παιδί, ο πατέρας έχει μια λίστα με τις χώρες οι οποίες του έχουν ανατεθεί. Έτσι, κάθε φορά που αναθέτει μια χώρα σε ένα παιδί, την εισάγει και στην αντίστοιχη λίστα.
- Στην συνέχεια, ξεκινάει να διαβάζει τα bloomFilters που του στέλνουν τα παιδία. Όπως αναφέρθηκε και παραπάνω, διαβάζει μόνο από όσα pipes περιέχουν δεδομένα, χωρίς να μπλοκάρει περιμένοντας "αργά" παιδιά. Ο πατέρας έχει μια λίστα από bloomFilters για κάθε παιδί. Κάθε φορά που διαβάζει ένα bloomFilter από ένα παιδί, το εισάγει στην αντίστοιχη λίστα.
- Μόλις διαβαστούν όλα τα bloomFilters από όλα τα παιδία, ο travelMonitor είναι έτοιμος να δεχθεί εντολές από τον χρήστη. Επομένως μέσα σε μια επαναληπτική διαδικασία κάνει τα εξής:
	- Περιμένει να διαβάσει κάποια εντολή από τον χρήστη.
	- Μόλις διαβαστεί η εντολή, ελέγχει αν έχει ληφθεί κάποιο σήμα όσο περίμενε την εντολή από τον χρήστη.
	  Ο έλεγχος αυτός γίνεται μέσω των flags τα οποία κάνουν set οι signal hanlders.
	  Αν κάποιο από τα flags είναι set τότε εκτελείται η αντίστοιχη λειτουργία που εξηγείται παρακάτω ανάλογα με το signal που λήφθηκε.
	- Στην συνέχεια εκτελεί την εντολή η οποία του "έδωσε" ο χρήστης. H κάθε λειτουργία εξηγείται ξεχωριστά στην συνέχεια.

Αν o travelMonitor λάβει ένα SIGINT ή SIGQUIT signal:
Καλείται ο αντίστοιχος signal handler ο οποίος θέτει το terminated flag ίσο με 1. Όταν ο travelMonitor διαβάσει την επόμενη εντολή από τον χρήστη (και πριν την εκτελέσει) ελέγχει αν το terminated flag είναι ίσο με 1. Εφόσον έχει γίνει ίσο με 1, ο travelMonitor κάνει break από την επαναληπτική διαδικασία, στέλνει ένα SIGKILL σε όλα τα παιδιά μέσω της συνάρτησης killChildren και τον πίνακα pidArray ο οποίος περιέχει τα process ids όλων των παιδιών. Στην συνέχεια γράφει στο logFile τις πληροφορίες που περιγράφονται στην εκφώνηση (μέσω της συνάρτησης writeLogFile),  περιμένει όλα τα παιδιά να τερματίσουν, αποδεσμεύει όλη την μνήμη που έχει δεσμεύσει δυναμικά και τέλος τερματίζει.

Αν o travelMonitor λάβει ένα SIGCHLD signal:
Καλείται ο αντίστοιχος signal handler ο οποίος θέτει το childTerminated flag ίσο με 1. Όταν ο travelMonitor διαβάσει την επόμενη εντολή από τον χρήστη (και πριν την εκτελέσει) ελέγχει αν το terminated flag είναι ίσο με 1. Εφόσον έχει γίνει ίσο με 1, καλεί την συνάρτηση restoreChildren έτσι ώστε να δημιουργήσει να νέα παιδιά τα οποία θα αντικαταστήσουν όσα τερματίστηκαν. Για κάθε ένα παιδί που τερμάτισε, δημιουργεί ένα νέο (μέσω fork) και ακολουθεί την διαδικασία αρχικοποίησης που περιγράφηκε παραπάνω. Δηλαδή, αρχικά του στέλνει το bufferSize και το bloomSize, στην συνέχεια του στέλνει τις χώρες που είχαν ανατεθεί στο αντίστοιχο παιδί που τερματίστηκε και στην συνέχεια περιμένει να διαβάσει τα bloomFilters τα οποία θα του στείλει το νέο αυτό παιδί. Επίσης, αντικαθιστά (στο πίνακα με τα process ids των παιδιών) το process id του "παλιού" παιδιού με αυτό του νέου.
Σε περίπτωση που παραπάνω από ένα παιδί τερματίστηκε, η διαδικασία αυτή εκτελείται επαναληπτικά.
Έτσι τελικά έχουν αντικατασταθεί όλα τα παιδιά που τερματίστηκαν με νέα, και η εντολή του χρήστη μπορεί να εκτελεστεί κανονικά.

!!! ΣΗΜΑΝΤΙΚΗ ΠΑΡΑΤΗΡΗΣΗ !!! :
Όταν ο travelMonitor λάβει ένα σήμα SIGINT,SIGQUIT,SIGCHLD τότε οι αντίστοιχες λειτουργίες θα εκτελεστούν αφού δοθεί η επόμενη εντολή (και πριν εκτελεστεί η αντολή αυτή). Αυτό συμβαίνει καθώς ο travel monitor είναι μπλοκαρισμένος περιμένοντας εντολή από τον χρήστη. Επέλεξα την "τακτική" αυτή καθώς θέλουμε οι signal handlers να είναι όσο πιο μικροί γίνεται. Στην υλοποίηση μου είναι μόλις μια γραμμή.
Έτσι αν ληφθεί ένα σήμα όσο ο travel monitor περιμένει μια εντολή, καλείται ο signal handler, θέτει το αντίστοιχο flag σε 1 και συνεχίζει να περιμένει εντολή από τον χρήση. Μόλις διαβαστεί μια εντολή (και πρίν γίνει οποιαδήποτε άλλη λειτουργία) ελέγχονται τα flags και εκτελούνται οι αντίστοιχες συναρτήσεις αν έχει ληφθεί κάποιο σήμα.
Για παράδειγμα, αν ένα child τερματίσει όσο ο travel monitor περιμένει μια εντολή, τότε μόλις δοθεί η επόμενη εντολή και πριν εκτελεστεί, ελέγχεται το childTerminated flag και καλείται η αντίστοιχη συνάρτηση έτσι ώστε να αντικατασταθεί το παιδί που τερματίστηκε. Αφού αντικατασταθεί το παιδί, συνεχίζεται κανονικά η εκτέλεση, δηλαδή εκτελείται η εντολή που δόθηκε από τον χρήστη.
Αν θέλουμε απλά να δοκιμάσουμε αν ο travel χειρίζεται σωστά τα signals μπόρουμε απλά να δώσουμε μια λανθασμένη εντολή (η ένα σκέτο \n) έτσι ώστε να ξεμπλοκάρει προσωρινά ο travel monitor και να εκτελεστούν οι αντίστοιχες λειτουργίες.

Monitors:
----------
- Αρχικά μέσω της sigaction ορίζει τους κατάλληλους signal handlers για τα σήματα SIGUSR1 και SIGINT/SIGQUIT.
- Ανοίγει τα 2 pipes τα οποία έλαβε σαν ορίσματα.
- Στην συνέχεια διαβάζει το bufferSize και το bloomSize που το έστειλε ο travelMonitor.
- Διαβάζει τις χώρες τις οποίες του έχει αναθέσει ο travelMonitor και τις αποθηκεύει σε μία λίστα.
- Για κάθε μια χώρα, με την χρήση της βιβλιοθήκης dirent.h βρίσκει τα αρχεία που αντιστοιχούν στην χώρα αυτή και διαβάζει το κάθε αρχείο. Για κάθε ένα αρχείο, μέσω της συνάρτησης που είχε υλοποιηθεί για την πρώτη εργασία, διαβάζει τις εγγραφές του αρχείου και εκχωρεί κάθε εγγραφή στις δομές δεδομένων.
- Στην συνέχεια στέλνει όλα τα bloomFilters στον travelMonitor. Πριν από κάθε bloomFilter στέλνεται πρώτα το όνομα του ιού στο οποίο αντιστοιχεί το bloomFilter έτσι ώστε να γνωρίζει ο travelMonitor σε ποιον ιό αναφέρεται το κάθε bloomFilter.
- Τέλος, το Monitor είναι έτοιμο να δεχθεί αιτήματα από τον travelMonitor. Επομένως μέσα σε μια επαναληπτική διαδικασία κάνει τα εξής:
	- Ελέγχει (μέσω της poll) αν του έχει στείλει κάποια εντολή ο travelMonitor.
	- Αν δεν του έχει στείλει τότε ελέγχει αν έχει ληφθεί κάποιο σήμα.
	  Ο έλεγχος αυτός γίνεται μέσω των flags τα οποία κάνουν set οι signal hanlders.
	  Αν κάποιο από τα flags είναι set τότε εκτελείται η αντίστοιχη λειτουργία που εξηγείται παρακάτω ανάλογα με το signal που λήφθηκε.
	- Αν του έχει στείλει τότε εκτελεί την εντολή αντίστοιχη εντολή και δίνει την αντίστοιχη απάντηση στον travelMonitor.
	  H κάθε λειτουργία εξηγείται ξεχωριστά στην συνέχεια.

Αν ένα Monitor λάβει ένα SIGINT ή SIGQUIT signal:
Καλείται ο αντίστοιχος signal handler ο οποίος θέτει το writeLog flag ίσο με 1. Όταν το monitor ελέγξει το flag και είναι ίσο με 1 τότε καλεί την συνάρτηση writeLogFile η οποία δημιουργεί ένα logFile, γράφει σε αυτό τις πληροφορίες που περιγράφονται στην εκφώνηση και επιστρέφει. Στην συνέχεια επαναφέρεται το writeLog flag σε 0.

Αν ένα Monitor λάβει ένα SIGUSR1 signal:
Καλείται ο αντίστοιχος signal handler ο οποίος θέτει το readNewFile flag ίσο με 1. Όταν το monitor ελέγξει το flag και είναι ίσο με 1 τότε καλεί την συνάρτηση readNewFiles η οποία είναι υπεύθυνη για να διαβάσει τα νέα αρχεία. Κάθε monitor έχει μονίμως σε μια λίστα τα ονόματα των αρχείων τα οποία έχει ήδη διαβάσει και των οποίων οι εγγραφές έχουν εισαχθεί στην βάση δεδομένων. Επομένως η readNewFile για κάθε ένα αρχείο που βρίσκεται στα directories που αντιστοιχούν στο Monitor, ελέγχει αν υπάρχει κάποιο αρχείο το οποίο δεν περιέχεται στην λίστα αυτήν. Σε περίπτωση που βρει κάποιο τέτοιο αρχείο, διαβάζει τις εγγραφές του και τις εισάγει στην βάση δεδομένων.
Τέλος στέλνει τα ανανεωμένα bloomFilters στον travelMonitor και επαναφέρει το readNewFile flag σε 0.
Σε περίπτωση που δεν βρέθει κανένα νέο αρχείο, τότε το Monitor δεν στέλνει κανένα bloomFilter στον travelMonitor καθώς δεν προστέθηκε κανένα νέο δεδομένο και κατ' επέκταση δεν τροποποιήθηκε κανένα bloomFilter (δηλαδή τα bloomFilters που ήδη έχει ο πατέρας παραμένουν "ενημερωμένα").


Εντολές εφαρμογής:
------------------

 /travelRequest citizenID date countryFrom countryTo virusName
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Αρχικά το travelMonitor βρίσκει σε ποιό monitor έχει ανατεθεί η χώρα countryFrom (έστω monitorX).
Στην συνέχεια ελέγχει αν το citizenID έχει εισαχθεί στο bloomFilter που του έστειλε το monitorX για τον ιό virusName.
Αν η απάντηση είναι αρνητική τότε το αίτημα απορρίπτεται και εκτυπώνεται το αντίστοιχο μήνυμα.
Αλλιώς ο travelMonitor "ρωτάει" το monitorX για το αν έχει εμβολιαστεί ο citizenID, δηλαδή στέλνει /travelRequest (έτσι ώστε να καταλάβει το monitor για τι αίτημα πρόκεται) ακολουθούμενο από το citizenID και το virusName.
Αφού διαβάσει τα παραπάνω μηνύματα, το monitorX, ελέγχει εάν ο πολίτης έχει εμβολιαστεί για τον ιό αυτό, δηλαδή ελέγχει εάν ο πολίτης υπάρχει στην vaccinated_skipList του ιού virusName (ακριβώς όπως κάναμε και στην εργασία 1). Σε περίπτωση που έχει εμβολιαστεί, στέλνει στον πατέρα το μήνυμα YES ακολουθούμενο από την ημερομηνία εμβολιασμού. Σε περίπτωση που δεν έχει εμβολιαστεί, τότε το monitorX στέλνει ΝΟ. Στην πρώτη περίπτωση, το monitorX περιμένει απάντηση από τον πατέρα έτσι ώστε να μάθει αν το αίτημα τελικά εγκρίθηκε ή όχι (και να αυξήσει κατάλληλα τους accepted/rejected counters).
O travelMonitor διαβάζει την απάντηση του monitorX. Στην περίπτωση που είναι YES, ελέγχει εάν η ημερομηνία εμβολιασμού είναι εντός 6 μηνών από το date. Εάν είναι, τότε το αίτημα εγκρίνεται ενώ αν δεν είναι τότε απορρίπτεται. Και στις 2 περιπτώσεις, στέλνει την αντίστοιχη ενημέρωση στο monitorX, έτσι ωστε να αυξήσει κατάλληλα τον αντίστοιχο counter accepted/rejected. Αν ο monitorX απάντησε NO, τότε το αίτημα απορρίπτεται.
Κάθε φορά τόσο ο travelMonitor όσο και το MonitorX (δηλαδή το monitor που διαχειρίζεται την χώρα countryFrom) αυξάνουν κατάλληλα τους counters που κρατάνε σχετικά με τα αιτήματα που δέχτηκαν.

Παρατήρηση: Αν το travel date που δόθηκε στην εντολή είναι πριν την ημερομηνία εμβολιασμού του πολίτη τότε εκτυπώνεται REQUEST REJECTED – YOU ARE NOT VACCINATED, δηλαδή σαν να μην έχει εμβολιαστεί ο πολίτης (όπως προτάθηκε και στο piazza).

 /travelStats virusName date1 date2 [country]
- - - - - - - - - - - - - - - - - - - - - - - - -
Κάθε φορά που ο travelMonitor λαμβάνει ένα αίτημα μέσω της /travelRequest αποθηκεύει τις πληροφορίες για κάθε αίτημα σε μια λίστα. Δηλαδή έχουμε μια λίστα με όλα τα αιτήματα τα οποία έχει δεχθεί ο travelMonitor. Για κάθε ένα αίτημα αποθηκεύεται ο ιός τον οποίο αφορά το αίτημα, η χώρα την οποία αφορά το αίτημα (countryTo), η ημερομηνία του ταξιδιού που αφορά το αίτημα καθώς και το αν έγινε αποδεκτό ή όχι.
Επομένως για να υπολογίσουμε τα στατιστικά, ελέγχουμε κάθε αίτημα της λίστας για το αν αφορά τον δοθέντα ιό, εάν βρίσκεται ενδιάμεσα στις δοθέντες ημερομηνίες και αν αφορά την δοθείσα χώρα προορισμού (εάν έχει δοθεί). Εάν πληρούνται αυτές οι προϋποθέσεις για ένα αίτημα, αυξάνουμε έναν συνολικό counter καθώς και έναν accepted/rejected counter ανάλογα με το αν το αίτημα αυτό είχε γίνει αποδεκτό η όχι.
Αφού ελέγξουμε όλα τα αιτήματα της λίστας, έχουμε υπολογίσει τα στατιστικά που θέλουμε, τα οποία και τυπώνουμε.
Το όρισμα country (αν αυτό έχει δοθεί) αντιστοιχεί στο όρισμα countryTo των αιτημάτων /travelRequest (όπως έχει αναφερθεί και στο piazza).
Τα στατιστικά που εκτυπώνονται είναι:
- Αν έχει δόθεί το country: Ο συνολικός αριθμός των travelRequests που έγιναν και περιέιχαν σαν countryTo την δοθείσα χώρα και το date (ημερομηνία του ταξιδιού) ήταν ανάμεσα στα date1 και date2. Επίσης, το πόσα από αυτά έγιναν αποδεκτά και πόσα όχι.
- Αν δεν έχει δόθεί το country: Ο συνολικός αριθμός των travelRequests που έγιναν και το date (ημερομηνία του ταξιδιού) ήταν ανάμεσα στα date1 και date2 καθώς και το πόσα από αυτά έγιναν αποδεκτά και πόσα όχι.

/addVaccinationRecords country
- - - - - - - - - - - - - - - -
Αρχικά το travelMonitor βρίσκει σε ποιό monitor έχει ανατεθεί η χώρα country (έστω monitorX).
Του στέλνει ένα σήμα SIGUSR1, και στην συνέχεια περιμένει να διαβάσει τα ανανεωμένα bloomFilters που θα του στείλει το monitorX τα οποία και αποθηκεύει στην λίστα με τα bloomFilters που αντιστοιχεί στο monitorX.
Σε περίπτωση που το monitorX δεν βρει κανένα νέο αρχείο τότε δεν στέλνει κανένα bloomFilter καθώς δεν προστέθηκε κανένα νέο δεδομένο σε αυτά. Αντίστοιχα, σε αυτήν την περίπτωση, ο πατέρας δεν διαβάζει κανένα bloomFIlter, και κρατάει αυτά που έχει ήδη.
Το Monitor μόλις λάβει το σήμα USR1, εκτελεί τις λειτουργίες που περιγράφονται παραπάνω.

/searchVaccinationStatus citizenID
- - - - - - - - - - - - - - - - - -
Το travelMonitor στέλνει σε όλα τα monitors /searchVaccinationStatus ακολουθούμενο από το citizenID.
Χρησιμοποιόντας την poll σε μια επαναληπτική διαδικασία διαβάζει τις απαντήσεις των παιδιών.
Όλα τα παιδιά τα οποία δεν βρίσκουν καμία εγγραφή για τον citizen απαντάνε το προκαθορισμένο μήνυμα ENDOF/SEARCHVACCINATIONSTATUS, και έτσι ο travelMonitor δεν περιμένει πια άλλη απάντηση από αυτά τα monitors.
To monitor το οποίο έχει πληροφορίες για τον citizen, στέλνει μέσω μηνυμάτων τις πληροφορίες αυτές τις οποίες και εκτυπώνει ο travel Monitor. Μόλις στείλει όλες τις πληροφορίες, στέλνει και αυτό ENDOF/SEARCHVACCINATIONSTATUS.
Όταν όλα τα monitors έχουν στείλει το μήνυμα ENDOF/SEARCHVACCINATIONSTATUS, τότε η διαδικασία ολοκληρώνεται καθώς έχουν διαβαστεί και εκτυπωθεί όλες οι πληροφορίες για τον citizen.


/exit
- - - -
Εάν ο χρήστης δώσει /exit, το terminated flag γίνεται ίσο με 1 και ακολουθείται η ίδια διαδικασία με αυτήν που περιγράφεται για όταν ο travelMonitor λαμβάνει ένα σήμα SIGINT/SIGQUIT


-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*

Bash Script
------------
- Αρχικά γίνεται έλεγχος των ορισμάτων. Αρχικά ελέγχεται αν ο αριθμός των ορισμάτων είναι σωστός, αν το inputFile υπάρχει, αν το input_dir δεν υπάρχει και αν numFilesPerDirectory είναι αριθμός. Αν κάτι από αυτα δεν ισχύει τότε εκτυπώνεται μήνυμα λάθους και η εκτέλεση τερματίζεται.
- Στην συνέχεια, "σκανάρεται" το αρχείο και εντοπίζονται όλες οι χώρες οι οποίες περιέχονται σε αυτό. Αποθηκεύω τις χώρες που εμφανίζονται στο αρχείο σε έναν πίνακα.
- Δημιουργείται το input_dir, μέσα σε αυτό δημιουργείται ένα directory για κάθε μία χώρα που εντοπίσαμε στο inputFile. Σε κάθε directory κάθε χώρας δημιουργούνται numFilesPerDirectory άδεια αρχεία με ονόματα country-n.txt όπου 1 <= n <= numFilesPerDirectory.
- Για κάθε μια χώρα έχουμε έναν counter ο οποίος δείχνει σε ποιό αρχείο θα μπει η επόμενη εγγραφή που θα συναντίσουμε από αυτήν την χώρα. Για παράδειγμα αν ο counter είναι 2 για την χώρα Greece η επόμενη εγγραφή που αφορά την χώρα Greece θα μπει στο αρχείο Greece-2.txt. Κάθε φορά που βάζουμε μια εγγραφή σε ένα αρχείο ο counter αυτός αυξάνεται κατά ένα, έτσι ώστε η επόμενη εγγραφή να μπει στο επόμενο αρχείο. Όταν ο counter ξεπεράσει το numFilesPerDirectory, επαναφέρεται στο 1 έτσι ώστε η επόμενη εγγραφή να μπει στο αρχείο country-1.txt. Με την χρήση αυτών των counter επιτυγχάνεται η Round Robin κατανομή των εγγραφών στα αρχεία.
- Με μια επαναληπτική διαδικασία, διαβάζουμε μια προς μια τις εγγραφές του inputFile και χρησιμοποιώντας τους counters που περιγράφηκαν παραπάνω την εισάγουμε στο κατάλληλο αρχείο της αντίστοιχης χώρας που αφορά η εγγραφή.
