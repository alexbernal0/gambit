//#
//# FILE: extform.cc -- Implementation of extensive form data type
//#
//# $Id$
//#

#include "extform.h"
#include "node.h"
#include "outcome.h"
#include "gmapiter.h"

static int generator = 0;

//---------------------------------------------------------------------------
//                 BaseExtForm: Constructors and Destructors
//---------------------------------------------------------------------------

BaseExtForm::BaseExtForm(void) 
  : players(0, 0), nodes((NodeSet *) 0), outcomes((Outcome *) 0)
{ }

BaseExtForm::~BaseExtForm()
{ }

//---------------------------------------------------------------------------
//                  BaseExtForm: Protected auxiliary functions
//---------------------------------------------------------------------------

void BaseExtForm::AddPlayer(int p)
{
  gSparseSetIter<NodeSet *> iter(nodes);
  for (iter.GoFirst(); !iter.PastEnd(); iter++)
    iter.GetValue()->AddPlayer(p);

  gSparseSetIter<Outcome *> oter(outcomes);
  for (oter.GoFirst(); !oter.PastEnd(); oter++)
    oter.GetValue()->SetNumPlayers(p);

  while (players.Last() < p)   players.Expand(1);
}

int BaseExtForm::CreateInfoset(int p, int game, int kids)
{
  return nodes(game)->CreateInfoset(p, kids);
}

//
// Delete a terminal node, and return the new identification of its parent
//
Node BaseExtForm::DeleteTerminalNode(const Node &n)
{
  Node parent;

  nodes(n.GetGame())->DeleteNode(n, parent);
  return parent;
}

//
// Delete the subtree rooted at n, and return the new identification of
// n's parent
//
Node BaseExtForm::DeleteSubtree(Node n)
{
  while (nodes(n.GetGame())->NumChildren(n))
    n = DeleteSubtree(nodes(n.GetGame())->GetChildNumber(n, 1));

  return DeleteTerminalNode(n);
}

//---------------------------------------------------------------------------
//                BaseExtForm: Title access and manipulation
//---------------------------------------------------------------------------

void BaseExtForm::SetTitle(const gString &s)
{
  title = s;
}

const gString &BaseExtForm::GetTitle(void) const
{
  return title;
}

//---------------------------------------------------------------------------
//            BaseExtForm: Data access -- general information
//---------------------------------------------------------------------------

int BaseExtForm::NumGames(void) const
{
  return nodes.Length();
}

//
// Returns the total number of nodes in all subgames
//
int BaseExtForm::NumNodes(void) const
{
  int total = 0;
  gSparseSetIter<NodeSet *> iter(nodes);
  for (iter.GoFirst(); !iter.PastEnd(); iter++)
    total += iter.GetValue()->NumNodes();
  return total;
}

//
// Returns the total number of nodes in a subgame
//
int BaseExtForm::NumNodes(int game) const
{
  if (nodes.IsDefined(game))
    return nodes(game)->NumNodes();
  else
    return 0;
}

//
// Returns the number of nodes at which player 'pl' has the decision in
// subgame number 'game'
//
int BaseExtForm::NumNodes(int game, int pl) const
{
  if (nodes.IsDefined(game))
    return nodes(game)->NumNodes(pl);
  else
    return 0;
}

//
// Returns the number of nodes in the information set identified by
// 'game', 'pl', and 'iset'
//
int BaseExtForm::NumNodes(int game, int pl, int iset) const
{
  if (nodes.IsDefined(game))
    return nodes(game)->NumNodes(pl, iset);
  else
    return 0;
}

//
// Returns the number of (personal) players defined in the game
//
int BaseExtForm::NumPlayers(void) const
{
  return players.Last();
}

//
// Returns the number of information sets controlled by player 'pl' in
// subgame 'game'
//
int BaseExtForm::NumInfosets(int game, int pl) const
{
  if (nodes.IsDefined(game))
    return nodes(game)->NumInfosets(pl);
  else
    return 0;
}

int BaseExtForm::NumActions(int game, int pl, int iset) const
{
  if (nodes.IsDefined(game))
    return nodes(game)->NumActions(pl, iset);
  else
    return 0;
}

//
// Returns the number of outcomes defined
//
int BaseExtForm::NumOutcomes(void) const
{
  return outcomes.Length();
}

//---------------------------------------------------------------------------
//                    BaseExtForm: Data access -- Nodes
//---------------------------------------------------------------------------

//
// Returns the root node of subgame number 'game'
//
Node BaseExtForm::RootNode(int game /* = 1 */) const
{
  if (nodes.IsDefined(game))
    return nodes(game)->RootNode();
  else
    return Node();
}

//
// Returns the parent node of the given node
//
Node BaseExtForm::GetParent(const Node &n) const
{
  int game = n.GetGame();

  if (nodes.IsDefined(game) && nodes(game)->IsMember(n))
    return nodes(game)->GetParent(n);
  else
    return Node();
}

//
// Returns the number of children of a given node
//
int BaseExtForm::NumChildren(const Node &n) const
{
  int game = n.GetGame();

  if (nodes.IsDefined(game) && nodes(game)->IsMember(n))
    return nodes(game)->NumChildren(n);
  else
    return 0;
}

//
// Returns the 'number'th child of a given node
//
Node BaseExtForm::GetChildNumber(const Node &n, int number) const
{
  int game = n.GetGame();

  if (nodes.IsDefined(game) && nodes(game)->IsMember(n))
    return nodes(game)->GetChildNumber(n, number);
  else
    return Node();
}

//
// Returns all the children of a given node
//
gBlock<Node> BaseExtForm::GetChildren(const Node &n) const
{
  int game = n.GetGame();

  if (nodes.IsDefined(game) && nodes(game)->IsMember(n))
    return nodes(game)->GetChildren(n);
  else
    return gBlock<Node>();
}

//
// Returns the prior sibling of a given node
//
Node BaseExtForm::GetPriorSibling(const Node &n) const
{
  int game = n.GetGame();

  if (nodes.IsDefined(game) && nodes(game)->IsMember(n))
    return nodes(game)->GetPriorSibling(n);
  else
    return Node();
}

//
// Returns the next sibling of a given node
//
Node BaseExtForm::GetNextSibling(const Node &n) const
{
  int game = n.GetGame();

  if (nodes.IsDefined(game) && nodes(game)->IsMember(n))
    return nodes(game)->GetNextSibling(n);
  else
    return Node();
}

//
// Returns true (nonzero) when a continuation subgame has been attached to
// the node
//
int BaseExtForm::HasSuccessorGame(const Node &n) const
{
  int game = n.GetGame();

  if (nodes.IsDefined(game) && nodes(game)->IsMember(n))
    return (nodes(game)->GetNextGame(n) != 0);
  else
    return 0;
}

//
// Returns the root node of the successor game attached to the node
//
Node BaseExtForm::GetSuccessorGameRoot(const Node &n) const
{
  int game = n.GetGame();

  if (nodes.IsDefined(game) && nodes(game)->IsMember(n) &&
      nodes(game)->GetNextGame(n) != 0)
    return nodes(nodes(game)->GetNextGame(n))->RootNode();
  else
    return Node();
}

//
// Returns nonzero if 'n' is a successor node of 'from'
//
int BaseExtForm::IsSuccessor(const Node &n, const Node &from) const
{
  if (n.GetGame() != from.GetGame())   return 0;

  int game = n.GetGame();

  return (nodes.IsDefined(game) && nodes(game)->IsMember(n) &&
	  nodes(game)->IsMember(from) && nodes(game)->IsPredecessor(from, n));
}

//
// Returns nonzero if 'n' is a predecessor node of 'of'
//
int BaseExtForm::IsPredecessor(const Node &n, const Node &of) const
{
  if (n.GetGame() != of.GetGame())   return 0;

  int game = n.GetGame();

  return (nodes.IsDefined(game) && nodes(game)->IsMember(n) &&
	  nodes(game)->IsMember(of) && nodes(game)->IsPredecessor(n, of));
}

//
// Returns the label of a node
//
gString BaseExtForm::GetNodeLabel(const Node &n) const
{
  int game = n.GetGame();

  if (nodes.IsDefined(game) && nodes(game)->IsMember(n))
    return nodes(game)->GetNodeName(n);
  else
    return "";
}

//
// Returns the number of the outcome associated with a node, if any
//
int BaseExtForm::GetOutcome(const Node &n) const
{
  int game = n.GetGame();
  
  if (nodes.IsDefined(game) && nodes(game)->IsMember(n))  {
    Outcome *outc = nodes(game)->GetOutcome(n);
    if (outc)   return outc->GetNumber();
    else   return 0;
  }
  else
    return 0;
}

//
// Returns the number of the continuation subgame associated with a node,
// if any
//
int BaseExtForm::GetNextGame(const Node &n) const
{
  int game = n.GetGame();

  if (nodes.IsDefined(game) && nodes(game)->IsMember(n))
    return nodes(game)->GetNextGame(n);
  else
    return 0;
}

void BaseExtForm::SetNextGame(const Node &n, int newgame)
{
  int game = n.GetGame();

  if (nodes.IsDefined(game) && nodes(game)->IsMember(n) &&
      nodes.IsDefined(newgame))
    nodes(game)->SetNextGame(n, newgame);
}

//---------------------------------------------------------------------------
//                  BaseExtForm: Player-related Operations
//---------------------------------------------------------------------------

gString BaseExtForm::GetPlayerLabel(int pl) const
{
  if (pl >= 0 && pl <= players.Last())
    return players[pl];
  return "";
}

void BaseExtForm::LabelPlayer(int pl, const gString &name)
{
  if (pl >= 0 && pl <= players.Last())
    players[pl] = name;
}

//---------------------------------------------------------------------------
//               BaseExtForm: Game element-related Operations
//---------------------------------------------------------------------------

//
// Return the label of a subgame
//
gString BaseExtForm::GetGameLabel(int game) const
{
  if (nodes.IsDefined(game))
    return nodes(game)->GetName();
  else
    return "";
}

//
// Set the label of a subgame
//
void BaseExtForm::LabelGame(int game, const gString &name)
{
  if (nodes.IsDefined(game))
    nodes(game)->SetName(name);
}

//
// Returns nonzero when the given subgame number is defined
//
int BaseExtForm::IsGameDefined(int game) const
{
  return nodes.IsDefined(game);
}

//--------------------------------------------------------------------------
//                    BaseExtForm: Node-related Operations
//--------------------------------------------------------------------------

//
// Append a decision node at a terminal node n, belonging to player 'player',
// and having 'child_count' children.  The node is made the only member of
// a newly-created information set, and the identity of the node is returned.
//
Node BaseExtForm::AddNode(const Node &n, int player, int child_count)
{
  int game = n.GetGame();

  if (!nodes.IsDefined(game) ||
      !nodes(game)->IsMember(n))  return Node(1, -1, 1, 1);
  if (!nodes(game)->IsTerminal(n))   return n;

  AddPlayer(player);

  int iset_no = CreateInfoset(player, game, child_count);
  Node ret(game, player, iset_no, 0);
  nodes(game)->MoveNode(n, ret);
  for (int i = 1; i <= child_count; i++)  
    nodes(game)->CreateNode(-1, CreateInfoset(-1, game, 0), ret);
  return ret;
}

//
// Set the label of a node
//
void BaseExtForm::LabelNode(const Node &n, const gString &s)
{
  int game = n.GetGame();

  if (nodes.IsDefined(game) && nodes(game)->IsMember(n))
    nodes(game)->SetNodeName(n, s);
}

//
// Insert a node in the tree at the location of the given node.  The new
// node belongs to player 'pl, has 'children' children, and is placed in
// a newly-created information set.  The existing node becomes the first
// (eldest) child of the newly-created node.  The identity of the newly-
// created node is returned.
//
Node BaseExtForm::InsertNode(const Node &n, int pl, int children)
{
  int game = n.GetGame();

  if (!nodes.IsDefined(game) ||
      !nodes(game)->IsMember(n))    return Node(1, -1, 1, 1);

  AddPlayer(pl);

  int iset = CreateInfoset(pl, game, children);

  Node new_node = nodes(game)->InsertNode(pl, iset, n);

  for (int i = 1; i < children; i++)
    nodes(game)->CreateNode(-1, CreateInfoset(-1, game, 0), new_node);

  return n;
}

//
// Delete a node in the tree.  The 'keep'th node is kept and replaces
// the deleted node in the tree; the subtrees rooted by the other children
// are deleted.
//
Node BaseExtForm::DeleteNode(const Node &n, int keep)
{
  int game = n.GetGame();

  if (!nodes.IsDefined(game) ||
      !nodes(game)->IsMember(n))    return Node(1, -1, 1, 1);
  if (nodes(game)->IsTerminal(n))    return n;

  Node foo(n), ret;
  
  for (int i = 1; i < keep; i++)
    foo = DeleteTerminalNode(DeleteTree(nodes(game)->GetChildNumber(foo, 1)));
  while (nodes(game)->NumChildren(foo) > 1) 
    foo = DeleteTerminalNode(DeleteTree(nodes(game)->GetChildNumber(foo, 2)));
  nodes(game)->DeleteInternalNode(foo, ret);
  return ret;
}

void BaseExtForm::SetOutcome(const Node &n, int outcome)
{
  if (!outcomes.IsDefined(outcome))  return;
  
  int game = n.GetGame();
  if (!nodes.IsDefined(game) || !nodes(game)->IsMember(n))  return;

  nodes(game)->SetOutcome(n, outcomes(outcome));
}

//--------------------------------------------------------------------------
//                  BaseExtForm: Infoset-related Operations
//--------------------------------------------------------------------------

//
// Moves 'new_node' from its current information set to the information set
// to which 'to_iset' belongs.  If this empties the information set, the
// information set is destroyed.  Returns the new identification of 'new_node'
//
Node BaseExtForm::JoinInfoset(const Node &new_node, const Node &to_iset)
{
  if (new_node.GetGame() != to_iset.GetGame())    return new_node;

  int game = new_node.GetGame();

  if (!nodes.IsDefined(game) ||
      !nodes(game)->IsMember(new_node) || !nodes(game)->IsMember(to_iset))
    return Node(1, -1, 1, 1);
  
  if (nodes(game)->NumChildren(new_node) != nodes(game)->NumChildren(to_iset))
    return new_node;

  if (new_node[1] == to_iset[1] && new_node[2] == to_iset[2])
    return new_node;

  Node ret(to_iset);
  
  if (nodes(game)->MoveNode(new_node, ret))
	//# take into account possible infoset renumbering...
    if (new_node[1] == ret[1] && new_node[2] < ret[2]) ret[2]--;
  return ret;
}

//
// Removes the node from its current information set and places it in a
// newly-created singleton information set owned by the same player.  Returns
// the new identification of 'n'.
//
Node BaseExtForm::LeaveInfoset(const Node &n)
{
  int game = n.GetGame();

  if (!nodes.IsDefined(game) ||
      !nodes(game)->IsMember(n))   return Node(1, -1, 1, 1);

  if (nodes(game)->NumNodes(n[1], n[2]) == 1)   return n;

  Node ret(game, n[1], CreateInfoset(n[1], game, nodes(game)->NumChildren(n)),
	   1);

      //# we already know this won't empty the infoset...
  nodes(game)->MoveNode(n, ret);

  return ret;
}

//
// Merges the nodes of the information set to which 'from' belongs into
// the information set to which 'into' belongs.  The newly-emptied information
// set is destroyed.  The new identification of 'from' is returned.
//
Node BaseExtForm::MergeInfoset(const Node &from, const Node &into)
{
  if (from.GetGame() != into.GetGame())   return from;

  int game = from.GetGame();

  if (!nodes.IsDefined(game) ||
      !nodes(game)->IsMember(from) || !nodes(game)->IsMember(into))
    return Node(1, -1, 1, 1);

  if (nodes(game)->NumChildren(from) != nodes(game)->NumChildren(into))
    return from;

  if (from[1] == into[1] && from[2] == into[2])
    return from;

  return nodes(game)->MoveNodes(from, into);
}

//
// Set the label of the information set to which node 'n' belongs
//
void BaseExtForm::LabelInfoset(const Node &n, const gString &label)
{
  int game = n.GetGame();

  if (nodes.IsDefined(game) && nodes(game)->IsMember(n))
    nodes(game)->SetInfosetName(n.GetPlayer(), n.GetInfoset(), label);
}

//
// Set the label of the information set identified by subgame, player, and
// information set number
//
void BaseExtForm::LabelInfoset(int game, int pl, int iset, const gString &s)
{
  nodes(game)->SetInfosetName(pl, iset, s);
}

//
// Return the label associated with the information set to which the node
// belongs.
//
gString BaseExtForm::GetInfosetLabel(const Node &n) const
{
  int game = n.GetGame();

  if (nodes.IsDefined(game) && nodes(game)->IsMember(n))
    return nodes(game)->GetInfosetName(n.GetPlayer(), n.GetInfoset());
  else
    return "";
}

//---------------------------------------------------------------------------
//                  BaseExtForm: Action-related Operations
//---------------------------------------------------------------------------

//
// Append a new action to all members of the information set specified
// by the game, player, and information set number
//
void BaseExtForm::AppendAction(int game, int pl, int iset)
{
  if (nodes.IsDefined(game))
    nodes(game)->AppendAction(pl, iset);
}

//
// Insert a new action for all members of the information set of which
// 'n' is a member.
//
void BaseExtForm::InsertAction(const Node &n, int where, int number)
{
  int game = n.GetGame();

  if (!nodes.IsDefined(game) || !nodes(game)->IsMember(n))   return;
  
  for (int i = 0; i < number; i++)
    nodes(game)->InsertAction(n.GetPlayer(), n.GetInfoset(), where + i);

      //# we have to remember to insert the branch in all members of the iset
  for (i = 0; i < number; i++)
    for (int j = 1; j <= nodes(game)->NumNodes(n[1], n[2]); j++)   
      nodes(game)->InsertChild(-1, CreateInfoset(-1, game, 0), 
			       Node(game, n[1], n[2], j), where + i);
}


//
// Deletes an action from all members of the information set of which
// 'n' is a member.  The subtrees rooted by each of the corresponding
// children are destroyed.  The new identification of 'n' is returned.
//
Node BaseExtForm::DeleteAction(const Node &n, int which)
{
  int game = n.GetGame();

  if (!nodes.IsDefined(game) || !nodes(game)->IsMember(n))
    return Node(1, -1, 1, 1);

  nodes(game)->RemoveAction(n.GetGame(), n.GetInfoset(), which);

  Node ret(n);

      //# remember to remove branch from all members of infoset
  for (int i = 1; i <= nodes(game)->NumNodes(ret[1], ret[2]); i++)  
    ret = DeleteTerminalNode(
	     DeleteTree(nodes(game)->GetChildNumber(Node(game,ret[1],ret[2],i),
					     which)));

  return ret;
}

//
// Sets the label of the 'act'th action at the information set of which
// 'n' is a member.
//
void BaseExtForm::LabelAction(const Node &n, int act, const gString &label)
{
  int game = n.GetGame();

  if (nodes.IsDefined(game) && nodes(game)->IsMember(n))
    nodes(game)->SetActionName(n.GetPlayer(), n.GetInfoset(), act, label);
}

//
// Sets the label of the 'act'th action at the information set specified
// by the game and player number
//
void BaseExtForm::LabelAction(int game, int pl, int iset, int act,
			      const gString &label)
{
  if (nodes.IsDefined(game) && pl >= 0 && pl <= players.Last() &&
      iset > 0 && iset <= nodes(game)->NumInfosets(pl))
    nodes(game)->SetActionName(pl, iset, act, label);
}

gString BaseExtForm::GetActionLabel(const Node &n, int act) const
{
  int game = n.GetGame();

  if (nodes.IsDefined(game) && nodes(game)->IsMember(n))
    return nodes(game)->GetActionName(n.GetPlayer(), n.GetInfoset(), act);
  return "";
}

//---------------------------------------------------------------------------
//                  BaseExtForm: Subtree-related Operations
//---------------------------------------------------------------------------

//
// Move the subtree rooted at 'src' to the terminal node 'dest', and return
// the new identification of 'src'.
//
Node BaseExtForm::MoveTree(Node src, Node dest)
{
  if (src.GetGame() != dest.GetGame())    return src;

  int game = src.GetGame();

  if (!nodes.IsDefined(game) ||
      !nodes(game)->IsMember(src) || !nodes(game)->IsMember(dest)) 
    return Node(1, -1, 1, 1);
  if (!nodes(game)->IsTerminal(dest))   return src;

  Node ret(src);
  nodes(game)->AdoptChildren(src, dest);
  nodes(game)->MoveNode(src, dest, 1);
  nodes(game)->MoveNode(Node(game, dest.GetPlayer(), dest.GetInfoset(), 1),
			ret);
 
  return ret;
}

//
// Copy the subtree rooted art 'src' to the terminal node 'dest', and return
// the new identification of 'src'.
//
Node BaseExtForm::CopyTree(Node src, Node dest)
{
  if (src.GetGame() != dest.GetGame())    return src;
  
  int game = src.GetGame();

  if (!nodes.IsDefined(game) ||
      !nodes(game)->IsMember(src) || !nodes(game)->IsMember(dest))
    return Node(1, -1, 1, 1);
  if (!nodes(game)->IsTerminal(dest) || nodes(game)->IsTerminal(src))
    return src;

  if (nodes(game)->IsPredecessor(src, dest))   return src;

  Node foo(game, src.GetPlayer(), src.GetInfoset(), 0);

  nodes(game)->MoveNode(dest, foo);

  for (int i = 1; i <= nodes(game)->NumChildren(src); i++)   {
    nodes(game)->CreateNode(-1, CreateInfoset(-1, game, 0), foo);
    CopyTree(nodes(game)->GetChildNumber(src, i),
	     nodes(game)->GetChildNumber(foo, i));
  }

  return src;
}

//
// Delete the subtree rooted at 'n', leaving 'n' as a terminal node.  Returns
// the new identification of 'n'.
//
Node BaseExtForm::DeleteTree(const Node &n)
{
  int game = n.GetGame();

  if (!nodes.IsDefined(game) || !nodes(game)->IsMember(n))
    return Node(1, -1, 1, 1);
  if (nodes(game)->IsTerminal(n))    return n;

  Node foo(n);

  while (nodes(game)->NumChildren(foo))
    foo = DeleteSubtree(nodes(game)->GetChildNumber(foo, 1));

  Node ret(game, -1, CreateInfoset(-1, game, 0), 1);

  nodes(game)->MoveNode(foo, ret);

  return ret;
}

//-------------------------------------------------------------------------
//                  BaseExtForm: Outcome-related operations
//-------------------------------------------------------------------------

//
// Delete an outcome
//
void BaseExtForm::RemoveOutcome(int outc)
{
  if (!outcomes.IsDefined(outc))   return;
  
  gSparseSetIter<NodeSet *> iter(nodes);
  for (iter.GoFirst(); !iter.PastEnd(); iter++)
    iter.GetValue()->ExpungeOutcome(outcomes(outc));

  delete outcomes.Remove(outc);
}

//
// Return the label of an outcome
//
gString BaseExtForm::GetOutcomeLabel(int outc) const
{
  if (outcomes.IsDefined(outc))
    return outcomes(outc)->GetName();
  else
    return "";
}

//
// Set the label of an outcome
//
void BaseExtForm::LabelOutcome(int outc, const gString &name)
{
  if (outcomes.IsDefined(outc))
    outcomes(outc)->SetName(name);
}

//
// Returns nonzero if the given outcome number is defined
//
int BaseExtForm::IsOutcomeDefined(int outc) const
{
  return outcomes.IsDefined(outc);
}

//-------------------------------------------------------------------------
//                    BaseExtForm: Variable Management
//-------------------------------------------------------------------------

int BaseExtForm::IsVariableDefined(const gString &name) const
{
  return nodes(1)->IsVariableDefined(name);
}

Node BaseExtForm::GetNodeVariable(const gString &name) const
{
  return nodes(1)->GetNodeVariable(name);
}

int BaseExtForm::SetNodeVariable(const gString &name, const Node &node)
{
  return nodes(1)->SetNodeVariable(name, node);
}

void BaseExtForm::RemoveNodeVariable(const gString &name)
{
  nodes(1)->RemoveNodeVariable(name);
}

gString BaseExtForm::GetUniqueVariable(void) const
{
  char buffer[20];

  sprintf(buffer, "_tmp%d", generator++);
  return buffer;
}


//---------------------------------------------------------------------------
//                 ExtForm<T>: Constructors and Destructors 
//---------------------------------------------------------------------------

//
// Construct the trivial extensive form game
//
template <class T> ExtForm<T>::ExtForm(void)
{
  players[0] = "CHANCE";
  nodes.Define(1, new GameEl<T>(1, 0, 0));
}


//
// Construct the extensive form game preparatory to reading in data
//
template <class T> ExtForm<T>::ExtForm(int)
{ }

//
// Clean up an extensive form game, deallocating all memory
//
template <class T> ExtForm<T>::~ExtForm()
{
  gSparseSetIter<NodeSet *> nodeiter(nodes);
  for (nodeiter.GoFirst(); !nodeiter.PastEnd();
       delete nodes.Remove(nodeiter.GetKey()));

  gSparseSetIter<Outcome *> outciter(outcomes);
  for (outciter.GoFirst(); !outciter.PastEnd();
       delete outcomes.Remove(outciter.GetKey()));
}

//---------------------------------------------------------------------------
//                ExtForm<T>: Game element-related Operations
//---------------------------------------------------------------------------

//
// Create a new subgame with the lowest undefined subgame number, and return
// the number of the new subgame
//
template <class T> int ExtForm<T>::CreateGame(void)
{
  int game = nodes.FirstVacancy();
  nodes.Define(game, new GameEl<T>(game, players.Last(), 0));
  return game;
}

//
// Create a new subgame with the given number
//
template <class T> int ExtForm<T>::CreateGame(int game, int from_file/*=0*/)
{
  if (nodes.IsDefined(game) || game <= 0)   return 0;
  nodes.Define(game, new GameEl<T>(game, players.Last(), from_file));
  return game;
}

//
// Delete a subgame
//
template <class T> void ExtForm<T>::RemoveGame(int game)
{
  if (!nodes.IsDefined(game))   return;

  delete nodes.Remove(game);

  gSparseSetIter<NodeSet *> iter(nodes);
  for (iter.GoFirst(); !iter.PastEnd(); iter++)
    iter.GetValue()->ExpungeGame(game);

  if (game == 1)
    nodes.Define(1, new GameEl<T>(1, players.Last()));
}

//--------------------------------------------------------------------------
//                  ExtForm<T>: Action-related Operations
//--------------------------------------------------------------------------


//
// Returns the vector of probabilities for each branch at a chance node
//
template <class T> gTuple<T> ExtForm<T>::GetActionProbs(const Node &n) const
{
  int game = n.GetGame();

  if (!nodes.IsDefined(game) ||
      !nodes(game)->IsMember(n) || n.GetPlayer() != 0)
    return gTuple<T>(1, 0);
  return ((GameEl<T> *) nodes(game))->GetActionProbs(n.GetInfoset());
}

//
// Returns the probability for a branch at a chance node
//
template <class T> T ExtForm<T>::GetActionProb(const Node &n, int act) const
{
  int game = n.GetGame();

  if (!nodes.IsDefined(game) ||
      !nodes(game)->IsMember(n) || n.GetPlayer() != 0)
    return -1.0;
  return ((GameEl<T> *) nodes(game))->GetActionProb(n.GetInfoset(), act);
}

//
// Set the vector of probabilities for each branch at a chance node
//
template <class T>
void ExtForm<T>::SetActionProbs(const Node &n, const gTuple<T> &probs)
{
  int game = n.GetGame();
  
  if (!nodes.IsDefined(game) ||
      !nodes(game)->IsMember(n) || n.GetPlayer() != 0)   return;
  ((GameEl<T> *) nodes(game))->SetActionProbs(n.GetInfoset(), probs);
}

//
// Set the vector of probabilities for each branch for a chance node
//
template <class T>
void ExtForm<T>::SetActionProbs(int game, int iset, const gTuple<T> &probs)
{
  ((GameEl<T> *) nodes(game))->SetActionProbs(iset, probs);
}


//---------------------------------------------------------------------------
//                   ExtForm<T>: Outcome-related Operations
//---------------------------------------------------------------------------

//
// Create a new outcome with the lowest unused outcome number, and return
// the number of the newly-created outcome
//
template <class T> int ExtForm<T>::CreateOutcome(void)
{
  int outc = outcomes.FirstVacancy();
  outcomes.Define(outc, new OutcomeVector<T>(outc, NumPlayers()));
  return outc;
}

//
// Create a new outcome with the given number
//
template <class T> int ExtForm<T>::CreateOutcome(int outc)
{
  if (outcomes.IsDefined(outc) || outc <= 0)    return 0;
  outcomes.Define(outc, new OutcomeVector<T>(outc, NumPlayers()));
  return outc;
}

//
// Returns the payoff vector associated with an outcome
//
template <class T> gVector<T> ExtForm<T>::GetOutcomeValues(int outc) const
{
  if (outcomes.IsDefined(outc))
    return ((OutcomeVector<T> *) outcomes(outc))->GetOutcomeVector();
  else
    return gVector<T>();
}

//
// Set the payoff vector associated with an outcome
//
template <class T>
void ExtForm<T>::SetOutcomeValues(int outc, const gVector<T> &vals)
{
  if (outcomes.IsDefined(outc))
    ((OutcomeVector<T> *) outcomes(outc))->SetOutcome(vals);
}

//
// Set the payoff associated with an outcome for one player
//
template <class T> void ExtForm<T>::SetOutcomeValue(int outc, int pl, T value)
{
  if (outcomes.IsDefined(outc) && pl <= players.Last())
    ((OutcomeVector<T> *) outcomes(outc))->SetOutcome(pl, value);
}


//--------------------------------------------------------------------------
//                      ExtForm<T>: Output Operations
//--------------------------------------------------------------------------

template <class T> void ExtForm<T>::WriteEfgFile(gOutput &f) const
{
  f << "{ \"" << title << "\" ";
  switch (Type())   {
    case DOUBLE:
      f << "FLOAT";
      break;
    case RATIONAL:
      f << "RATIONAL";
      break;
  }

  f << "\n\n{ ";
  for (int i = 0; i <= players.Last(); i++)
    f << '"' << players[i] << '"' << ' ';
  f << "}\n\n";
  
  f << "{ ";

  gSparseSetIter<Outcome *> out_iter(outcomes);

  int flag = 0;
  for (out_iter.GoFirst(); !out_iter.PastEnd(); out_iter++)   {
    if (flag)
      f << "\n  ";
    else
      flag = 1;
    f << "{ " << out_iter.GetKey() << ' ' 
      << ((OutcomeVector<T> *) out_iter.GetValue())->GetOutcomeVector()
      << " \"" << out_iter.GetValue()->GetName() << "\" }";
  }
  f << " }\n\n";

  gSparseSetIter<NodeSet *> game_iter(nodes);
  for (game_iter.GoFirst(); !game_iter.PastEnd(); game_iter++)   {
    GameEl<T> *game = (GameEl<T> *) game_iter.GetValue();

    f << "{ " << game_iter.GetKey() << " \"" << game->GetName() << '"' << '\n';
    f << "  {";
    for (i = 0; i <= game->NumPlayers(); i++)   {
      f << ((i == 0) ? " {" : "    {");
      for (int j = 1; j <= game->NumInfosets(i); j++)  {
	f << ((j == 1) ? " " : "\n      ");
	f << "{ \"" << game->GetInfosetName(i, j) << "\" {";
	for (int k = 1; k <= game->NumActions(i, j); k++)
	  f << " \"" << game->GetActionName(i, j, k) << '"';
	f << " }";
	
	if (i == 0)   { // chance player, print branch probs
	  f << " { ";
	  for (k = 1; k <= game->NumActions(i, j); k++)
	    f << game->GetActionProb(j, k) << ' ';
	  f << '}';
	}
	
	f << " }";
      }
      f << " }\n";
    }
    f << "  }\n\n";  

    game->WriteToFile(f);
  
    f << "}\n";
  }
  
  f << "\n}\n";
}

template <class T> void ExtForm<T>::DisplayTree(gOutput &f, int game) const
{
  if (nodes.IsDefined(game))
    nodes(game)->DisplayTree(f, nodes(game)->RootNode(), 1);
}

//----------------------------------------------------------------------------
//                      ExtForm<T>: Computing Payoffs
//----------------------------------------------------------------------------

#include "gdpvect.h"

template <class T>
T ExtForm<T>::Payoff(int pl, const gDPVector<T> &strategy) const
{
  return ((GameEl<T> *) nodes(1))->Payoff(pl, strategy);
}

template <class T> gPVector<int> ExtForm<T>::Dimensionality(void) const
{
  gTuple<int> foo(NumPlayers());
  for (int i = 1; i <= NumPlayers(); i++)
    foo[i] = NumInfosets(1, i);
  
  gPVector<int> bar(foo);
  for (i = 1; i <= NumPlayers(); i++)
    for (int j = 1; j <= NumInfosets(1, i); j++)
      bar(i, j) = NumActions(1, i, j);

  return bar;
}

template <class T> int ExtForm<T>::ProfileLength(void) const
{
  int sum = 0;

  for (int i = 1; i <= NumPlayers(); i++)
    for (int j = 1; j <= NumInfosets(1, i); j++)
      sum += NumActions(1, i, j);

  return sum;
}


template <class T> void ExtForm<T>::CondPayoff(const gDPVector<T> &profile,
					       gDPVector<T> &payoff) const
{
  ((GameEl<T> *) nodes(1))->CondPayoff(profile, payoff);
}

template <class T> void ExtForm<T>::Payoff(const gPVector<int> &profile,
					   gVector<T> &payoff) const
{
  ((GameEl<T> *) nodes(1))->Payoff(profile, payoff);
}

DataType ExtForm<double>::Type(void) const   { return DOUBLE; }
DataType ExtForm<gRational>::Type(void) const  { return RATIONAL; }

#ifdef __GNUG__
#define TEMPLATE template
#elif defined __BORLANDC__
#define TEMPLATE
#pragma option -Jgd
#endif   // __GNUG__, __BORLANDC__

TEMPLATE class ExtForm<double>;
TEMPLATE class ExtForm<gRational>;

