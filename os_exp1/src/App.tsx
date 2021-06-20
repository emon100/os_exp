import React, {useState, useReducer} from 'react';
import {CSSTransition, TransitionGroup} from 'react-transition-group'
import produce from 'immer';
import './App.css';

interface IProcess {
    pid:number;
    memory:number;
    timesliceRemaining:number;
}

interface IBlockingProcess extends IProcess{
    blockingCounter:number;
}

interface OSState {
    totalMemoryUsed: number;
    newProcesses: IProcess[];
    runningProcesses: IProcess[];
    readyProcesses: IProcess[];
    blockingProcesses: IBlockingProcess[];
    exitProcesses: IProcess[];
}

type NormalAction = {
    type: "NEXT_QUANTUM_NEED_BLOCK"| "NEXT_QUANTUM_WITHOUT_BLOCK"
}

type AddAction = {
    type: "ADD_NEW_PROC"
    nextProc: IProcess
}

type AdmitAction = {
    type: "ADMIT_PROC"
    pid: number
}

const MAX_CONCURRENT_RUNNING = 1;

function ProcessOps(props: { dispatch: React.Dispatch<AddAction | NormalAction> }) {
    const [pid, setPid] = useState(0);
    const {dispatch} = props;

    function add() {
        const timesliceRemaining = Math.ceil(Math.random()*9);
        const memory = Math.ceil(Math.random()*9);
        dispatch({type: "ADD_NEW_PROC", nextProc:{pid,memory,timesliceRemaining}});
        setPid(e => e + 1);
    }

    return (
        <div id="processOps">
            <button type="button" onClick={add}>
                添加进程
            </button>
            <button type="button" onClick={() => dispatch({type: "NEXT_QUANTUM_NEED_BLOCK"})}>
                下个时间片（运行进程将被堵塞）
            </button>
            <button type="button" onClick={() => dispatch({type: "NEXT_QUANTUM_WITHOUT_BLOCK"})}>
                下个时间片（运行进程不被堵塞）
            </button>
        </div>
    );
}

function BlockingProcess(props:any) {
    const {p: {pid, memory, timesliceRemaining, blockingCounter}, children, ...rest} = props;
    return (
        <div {...rest} style={{"--counter": blockingCounter}}>
            {pid}
            <div className="blockingCounter" title="阻塞还需几个时间片结束">
                <span className="tooltip" data-tooltip="阻塞还需几个时间片结束">
                    {blockingCounter}
                </span>
            </div>
            <div className="timesliceRemaining" title="还需运行几个时间片">
                <span className="tooltip" data-tooltip="还需运行几个时间片">
                    {timesliceRemaining}
                </span>
            </div>
            <div className="memoryCounter">
                <span className="tooltip" data-tooltip="使用了多少内存">
                    {memory}
                </span>
            </div>
        </div>
    )
}

function ProcessesView(props:any) {
    const {processes, title, children, ...rest} = props;
    return (
        <div className={`Processes`} {...rest}>
            <div className="processHeader">{children}</div>
            <TransitionGroup component={null} exit={false}>
                {
                    title === "blocking" ?
                        processes.map((p:IBlockingProcess) => (
                            <CSSTransition key={p.pid} timeout={500} classNames="process">
                                <BlockingProcess className="process blocking" p={p}/>
                            </CSSTransition>
                        ))
                        :
                        processes.map(({pid,memory,timesliceRemaining}:IProcess) => (
                            <CSSTransition key={pid} timeout={500} classNames="process" data-pid={pid}>
                                <div className={`process ${title}`}>
                                    {pid}
                                    <div className="timesliceRemaining" title="还需运行几个时间片">
                                        <span className="tooltip" data-tooltip="还需运行几个时间片">
                                            {timesliceRemaining}
                                        </span>
                                    </div>
                                    <div className="memoryCounter" title="使用了多少内存">
                                        <span className="tooltip" data-tooltip="使用了多少内存">
                                            {memory}
                                        </span>
                                    </div>
                                </div>
                            </CSSTransition>
                        ))
                }
            </TransitionGroup>
        </div>
    );
}


/*
                              ┌-------------------┐
                              │                   ↓
五状态   new--->ready<-->running--->blocking     exit
                    ↑                  │
                    └---------------- -┘

        blocking:blockingCounter-1
        running:go blocking or ready
        readying: some go running.
*/
function nextQuantum(draft: OSState, goBlock: boolean) {
    const {
        runningProcesses: runningProcs,
        readyProcesses: readyProcs
    } = draft;
    const blockingProcs = draft.blockingProcesses.filter(p => {
        --p.blockingCounter;
        const ok = p.blockingCounter > 0;
        if (!ok) {
            const {blockingCounter, ...okp} = p;
            readyProcs.push(okp);
        }
        return ok;
    });

    runningProcs.forEach(p => {
        --p.timesliceRemaining;
        if (p.timesliceRemaining > 0) {
            if (goBlock) {
                const bp = (p as IBlockingProcess);
                bp.blockingCounter = Math.ceil(Math.random() * 10);
                blockingProcs.push(bp);
            } else {
                readyProcs.push(p);
            }
        }else{
            draft.exitProcesses.push(p);
            draft.totalMemoryUsed-=p.memory;
        }
    });

    draft.blockingProcesses = blockingProcs;
    draft.runningProcesses = readyProcs.splice(0, MAX_CONCURRENT_RUNNING);
    draft.blockingProcesses = blockingProcs.sort((p1, p2) => p1.blockingCounter - p2.blockingCounter);
}

const OSScheduler = produce((draft:OSState, action:NormalAction|AddAction|AdmitAction) => {
    switch (action.type) {
        case "NEXT_QUANTUM_NEED_BLOCK":{
            nextQuantum(draft,true);
            return;
        }
        case "NEXT_QUANTUM_WITHOUT_BLOCK":{
            nextQuantum(draft,false);
            return;
        }
        case "ADD_NEW_PROC": {
            draft.newProcesses.push(action.nextProc);
            return;
        }
        case "ADMIT_PROC": {
            const idx = draft.newProcesses.findIndex(p=>p.pid===action.pid);
            const okProc = draft.newProcesses[idx];
            draft.newProcesses.splice(idx,1);
            draft.readyProcesses.push(okProc);
            draft.totalMemoryUsed+=okProc.memory;
            return;
        }
        default:
            throw new Error();
    }
});

const OSInitialState: OSState = {
    totalMemoryUsed: 0,
    newProcesses:[],
    runningProcesses: [],
    readyProcesses: [],
    blockingProcesses: [],
    exitProcesses:[]
};

function App() {
    const [processes, dispatch] = useReducer(OSScheduler, OSInitialState);

    function click2Admit({target}:React.SyntheticEvent){
        const pid = parseInt((target as any).dataset?.pid);
        dispatch({type: "ADMIT_PROC", pid});
    }

    return (
        <div className="App">
            <div>系统已用内存: {processes.totalMemoryUsed}</div>
            <ProcessesView processes={processes.newProcesses} title="new" onClick={click2Admit} >
                New
            </ProcessesView>
            <ProcessesView processes={processes.runningProcesses} title="running">
                Running
            </ProcessesView>
            <ProcessesView processes={processes.readyProcesses} title="ready">
                Ready
            </ProcessesView>
            <ProcessesView processes={processes.blockingProcesses} title="blocking">
                Blocking
            </ProcessesView>
            <ProcessesView processes={processes.exitProcesses} title="exit">
                Exit
            </ProcessesView>
            <ProcessOps {...{dispatch}}/>
        </div>
    );
}

export default App;
