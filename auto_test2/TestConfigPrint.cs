using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace AutoTestClient;


public class TestConfigPrint
{
    public static void Print(TestConfig config)
    {
        if (config.TaskConfigs == null || config.TaskConfigs.Count == 0)
        {
            Console.WriteLine("No tasks configured.");
            return;
        }

        Console.WriteLine($"Douumy Count: {config.DummyCount}");
        Console.WriteLine($"Douumy Start Number: {config.DummyStartNumber}");
        Console.WriteLine($"RemoteEndPoint: {config.RemoteEndPoint}");
        Console.WriteLine($"Scenario: {config.ScenarioName}");
        Console.WriteLine("Task Tree:");

        // 각 태스크를 시작점으로 하여 트리 출력
        HashSet<string> visitedTasks = new HashSet<string>();
        foreach (var task in config.TaskConfigs)
        {
            if (!visitedTasks.Contains(task.TaskName))
            {
                PrintTaskNode(task, config.TaskConfigs, "", true, visitedTasks, new HashSet<string>());
            }
        }
    }

    private static void PrintTaskNode(
        TaskConfig task,
        List<TaskConfig> allTasks,
        string indent,
        bool isLast,
        HashSet<string> visitedTasks,
        HashSet<string> currentPath)
    {
        // 현재 경로에 이미 있는 태스크라면 순환 참조를 표시하고 반환
        if (currentPath.Contains(task.TaskName))
        {
            Console.WriteLine($"{indent}└── {task.TaskName} (Circular Reference)");
            return;
        }

        visitedTasks.Add(task.TaskName);
        currentPath.Add(task.TaskName);

        // 현재 노드 출력
        string nodePrefix = isLast ? "└── " : "├── ";
        Console.Write($"{indent}{nodePrefix}{task.TaskName}");

        // 확률과 대기 시간 정보 출력
        if (task.NextTasks != null && task.NextTasks.Count > 0)
        {
            Console.WriteLine();
            string childIndent = indent + (isLast ? "    " : "│   ");

            for (int i = 0; i < task.NextTasks.Count; i++)
            {
                string nextTaskName = task.NextTasks[i];
                int probability = task.NextTaskProbabilityList[i];
                int minWaitTime = task.NextTaskWaitMinTimeMSList[i];
                int maxWaitTime = task.NextTaskWaitMaxTimeMSList[i];

                Console.WriteLine($"{childIndent}│ Prob: {probability}%, Wait: {minWaitTime}-{maxWaitTime}ms");

                // 다음 태스크 찾기
                var nextTask = allTasks.FirstOrDefault(t => t.TaskName == nextTaskName);
                if (nextTask != null)
                {
                    bool isLastChild = (i == task.NextTasks.Count - 1);
                    PrintTaskNode(nextTask, allTasks, childIndent, isLastChild, visitedTasks, currentPath);
                }
                else
                {
                    Console.WriteLine($"{childIndent}└── {nextTaskName} (Undefined Task)");
                }
            }
        }
        else
        {
            Console.WriteLine(" (End)");
        }

        currentPath.Remove(task.TaskName);
    }
}