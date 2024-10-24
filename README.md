# ngl-tbc

This is the ngl-tbc project.

# Core Principles

The main difference between a turn-based system compared to other games is their relationship with time and their expectations regarding the timing of player input. A turn-based battle can be modeled as a series of mutations to some domain-specific battle state over some duration. A Turn can be considered an abstract base unit of time, against which the average planned frequency of each Player's Actions are defined. Players perform Actions by providing domain-specific Commands at the Battle's request. The time between the provision of a Command and its execution as an Action is domain and situation-specific. An extremely common abstraction over Players and Actions is the concept of Units, which are discrete domain-specific entities that are said to perform Actions based on the Player's Commands.

## Turns

A Turn is the base unit of time, and all Actions are scheduled around this concept. Specifically, the planned frequency at which a Player may take Actions is defined in terms of the duration of a Turn. The specific scheduling rules are domain specific, but there are several common scheduling methods that most games will implement. The names used here are only descriptive, and do not correspond to any more widely spread nomenclature

- Alternation: Players take Actions in a consistent alternating scheme, like in checkers or chess
- Sequential: Players take an equal number of Actions per Turn each, but the order the Actions are taken in may change, like a dynamic initiative system in D&D
- Lapping: The planned Action frequency of some players spans multiple Turns, meaning there are Turns they are unable to take Actions. This is used in various games for things like balancing high power encounters by making them take Actions less frequently
- Continuous/ATB: Some systems are more concerned with each Player's Action frequency relative to one another, rather than in relation to the duration of a Turn. This can be modelled by making the granularity of a Turn very small, allowing each Player's Action frequency to be very granular and accurate. The relative timescale appears more and more continuous as the Turn duration approaches 0, so this system is considered to model "continuous" time. However since so many Turns in this model don't do anything other than bring a Player closer to taking an Action, they typically aren't implemented this way. This system is canonically known as the "Active Battle Time" system which was originally designed for Final Fantasy

## Action Resolution

Actions have a notion of occurring in an order at specific points in time. This means that it is conceptually possible to schedule multiple Actions to occur at the same point in time. There are basically two strategies for resolving Actions that have been scheduled to occur simultaneously

- Sequential Action Resolution: Actions scheduled to the same point in time are considered to still have an order, and are executed sequentially based on domain-specific scheduling/tiebreaking rules. This would be a common strategy to manage ties in ATB systems
- Simultaneous Action Resolution: Actions are intentionally scheduled to occur at the same point in time as a domain-modelling decision, and are not considered to be orderable when occurring at the same time. Instead some domain-specific rules will be applied to resolve the Actions that occur simultaneously. This strategy is the principle of Action resolution in so-called "WeGo" systems

## Command Provision

Players provide Commands to Battles in order to select an Action to take. Because Battles define their own notion of time, only Battles know when they require a Command from a Player, so communication to request a Command is always initiated by a Battle. The time between the provision of a Command and its interpretation and execution as an Action can vary arbitrarily, and systems may use a combination of timings, but common examples include

- Immediate Commands: Commands are requested as needed and interpreted and acted on immediately. This is the default for things like ATB systems
- Planning Phases: Turns may be split into a "planning phase" where Commands are requested from participating Players to be scheduled and applied during an "execution phase". Players won't necessarily know the order their corresponding Actions will be executed in, especially relative to each others, and they may not even know what the state of the battle will be when their Actions take place. Systems that schedule Commands like this may also make use of unplanned immediate Commands to resolve certain domain-specific situations
- Buffered Commands: Systems may allow buffering a Command an arbitrary number of Turns in the future either directly or through certain domain-specific behaviours

# Building and installing

See the [BUILDING](BUILDING.md) document.

# Contributing

See the [CONTRIBUTING](CONTRIBUTING.md) document.

# Licensing

<!--
Please go to https://choosealicense.com/licenses/ and choose a license that
fits your needs. The recommended license for a project of this type is the
GNU AGPLv3.
-->
